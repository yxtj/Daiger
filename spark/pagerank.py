#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

"""
This is an example implementation of PageRank. For more conventional use,
Please refer to PageRank implementation provided by graphx

Example Usage:
bin/spark-submit examples/src/main/python/pagerank.py data/mllib/pagerank_data.txt 10
"""
from __future__ import print_function

import re
import sys
from operator import add
import time

from pyspark.sql import SparkSession
from pyspark import SparkContext

def computeContribs(urls, rank):
    """Calculates URL contributions to the rank of other URLs."""
    num_urls = len(urls)
    for url in urls:
        yield (url, rank / num_urls)


def parseNeighbors(urls):
    """Parses a urls pair string into urls pair."""
    parts = re.split(r'\s+', urls)
    return parts[0], parts[1]

def parseNeighborList(line):
    t=line.split('\t')
    key=int(t[0])
    value=[int(v) for v in t[1].split(' ') if len(v)>0]
    return (key, value)

#def difference(x, y):
#   return x.join(y).mapValues(lambda p:abs(p[0]-p[1])).map(lambda pv: pv[1]).reduce(add)

if __name__ == "__main__":
    argc=len(sys.argv)
    if argc < 3 or argc > 7:
        print("Usage: pagerank <file> <iterations> [epsilon=1e-6] [damp-factor=0.8] [break-lineage=3] [output-file]", file=sys.stderr)
        exit(-1)
    infile=sys.argv[1]
    max_iteration=int(sys.argv[2])
    epsilon=float(sys.argv[3]) if argc > 3 else 1e-6
    damp=float(sys.argv[4]) if argc > 4 else 0.8
    damp_c=1-damp
    break_lineage=int(sys.argv[5]) if argc > 5 else 3
    outfile=sys.argv[6] if argc > 6 else ''

    # Initialize the spark context.
    sc = SparkContext()
    spark = SparkSession\
        .builder\
        .appName("PythonPageRank")\
        .getOrCreate()

    time0=time.time()
    # Loads in input file. It should be in format of:
    #     URL         neighbor URL
    #     URL         neighbor URL
    #     URL         neighbor URL
    #     ...
    
    # my format:
    # a\ta b c d 
    # b\td e f 
    # c\t a d
    # ...
    lines = spark.read.text(infile).rdd.map(lambda r: r[0])

    # Loads all URLs from input file and initialize their neighbors.
    #links = lines.map(lambda urls: parseNeighbors(urls)).distinct().groupByKey().cache()
    links = lines.map(lambda urls: parseNeighborList(urls)).cache()

    # Loads all URLs with other URL(s) link to from input file and initialize ranks of them to one.
    ranks = links.map(lambda url_neighbors: (url_neighbors[0], 1.0))
    progress = links.count()*1.0
    
    time1=time.time()   
    # Calculates and updates URL ranks continuously using PageRank algorithm.
    for iteration in range(max_iteration):
        time_iter=time.time()
        # Calculates URL contributions to the rank of other URLs.
        contribs = links.join(ranks).flatMap(
            lambda url_urls_rank: computeContribs(url_urls_rank[1][0], url_urls_rank[1][1]))

        # Re-calculates URL ranks based on neighbor contributions.
        ranks_new = contribs.reduceByKey(add).mapValues(lambda rank: rank * damp + damp_c)
        progress_new = ranks_new.map(lambda v: v[1]**2).reduce(add)
        diff = abs(progress_new - progress)
        if break_lineage != 0 and iteration != 0 and iteration % break_lineage == 0:
            ranks = sc.parallelize(ranks_new.collect())
        else:
            ranks = ranks_new
        progress = progress_new
        time_iter = time.time()-time_iter
        print("finish iteration: %d, progress: %f, improvement: %f, used time: %f" % (iteration, progress, diff, time_iter))
        if diff < epsilon:
            break
    
    time2=time.time()
    # Collects all URL ranks and dump them
    if len(outfile) == 0:
        print("Skip outputting")
    elif outfile == '#console':
        for (link, rank) in ranks.collect():
            print("%d\t%f" % (link, rank))
    else:
        ranks.saveAsTextFile(outfile)
        
    time3=time.time()
    
    print('iterations: %d' % min(iteration+1, max_iteration))
    print('loading time: %f' % (time1-time0))
    print('computing time: %f' % (time2-time1))
    print('dumping time: %f' % (time3-time2))
    
    spark.stop()
