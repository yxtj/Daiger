"""
Connected Component

Example Usage:
bin/spark-submit cc.py data/xxx.txt 0
"""
from __future__ import print_function

import re
import sys
from operator import add
import time
import math

from pyspark.sql import SparkSession
from pyspark import SparkContext

def computeContribs(neighbors, rank):
    for dst in neighbors:
        yield (dst, rank)

def parseNeighbors(urls):
    parts = re.split(r'\s+', urls)
    return parts[0], parts[1]

def parseNeighborList(line):
    t=line.split('\t')
    key=int(t[0])
    value=[int(v) for v in t[1].split(' ') if len(v)>0]
    return (key, value)

if __name__ == "__main__":
    argc=len(sys.argv)
    if argc < 2 or argc > 4:
        print("Usage: cc <file> [break-lineage=3] [output-file]", file=sys.stderr)
        exit(-1)
    infile=sys.argv[1]
    break_lineage=int(sys.argv[2]) if argc > 2 else 3
    outfile=sys.argv[3] if argc > 3 else ''

    # Initialize the spark context.
    sc = SparkContext()
    spark = SparkSession\
        .builder\
        .appName("PythonConnectedComponent")\
        .getOrCreate()

    time0=time.time()
    # my format:
    # a\ta b c d 
    # b\td e f 
    # c\t a d
    # ...
    lines = spark.read.text(infile).rdd.map(lambda r: r[0])

    # Loads all nodes from input file and initialize their neighbors
    nodes = lines.map(lambda l: parseNeighborList(l)).cache()
    n = nodes.count()

    # initialize cc to be their indices
    cc = nodes.map(lambda neighbors: (neighbors[0], neighbors[0]))
    progress = int((n-1)*n/2)
    
    time1=time.time()   
    # Calculates and updates cc continuously
    for iteration in range(n):
        time_iter=time.time()
        contribs = nodes.join(cc).flatMap(
            lambda k_list_sp: computeContribs(k_list_sp[1][0], k_list_sp[1][1]))
        # Re-calculates cc based on neighbor contributions.
        cc_new = contribs.reduceByKey(min).cache()
        progress_new = cc_new.map(lambda v: v[1]).reduce(add)
        diff = abs(progress_new - progress)
        # to cut down the long linage which greately slown down the process
        if break_lineage != 0 and iteration != 0 and iteration % break_lineage == 0:
            cc = sc.parallelize(cc_new.collect())
        else:
            cc = cc_new
        progress = progress_new
        time_iter = time.time()-time_iter
        print("finish iteration: %d, progress: %d, improvement: %d, used (second): %f" % (iteration, progress, diff, time_iter))
        if diff == 0:
            break
    
    time2=time.time()
    # Collects all URL cc and dump them
    if len(outfile) == 0:
        print("Skip outputting")
    elif outfile == '#console':
        for (link, rank) in cc.collect():
            print("%d\t%f" % (link, rank))
    else:
        cc.saveAsTextFile(outfile)
        
    time3=time.time()
    
    print('iterations: %d' % (iteration+1))
    print('loading time: %f' % (time1-time0))
    print('computing time: %f' % (time2-time1))
    print('dumping time: %f' % (time3-time2))
    
    spark.stop()
