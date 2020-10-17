"""
This is an example implementation of PageRank.

Example Usage:
bin/spark-submit pagerank.py data/pagerank_data.txt 10 1e-8
"""
from __future__ import print_function

import re
import os
import sys
from operator import add
import time

from pyspark.sql import SparkSession
from pyspark import SparkContext

def listFiles(fld, prefix):
    l = os.listdir(fld)
    l = [f for f in l if os.path.isfile(fld+'/'+f) and f.startswith(prefix)]
    ptn = prefix+'\\d+'
    l = [f for f in l if re.match(ptn, f)]
    return l

def loadFile(infile, prefix, opt_prefix=None):
    if os.path.isfile(infile):
        lines = spark.read.text(infile).rdd.map(lambda r: r[0])
    else:
        fl = listFiles(infile, prefix)
        if len(fl) == 0 and opt_prefix is not None:
            fl = listFiles(infile, opt_prefix)
        tmp = [spark.read.text(infile+'/'+f).rdd.map(lambda r: r[0]) for f in fl]
        lines = sc.union(tmp)
    return lines

def parseNeighborList(line):
    t=line.split('\t')
    key=int(t[0])
    value=[int(v) for v in t[1].split(' ') if len(v)>0]
    return (key, value)

def computeContribs(urls, rank):
    """Calculates URL contributions to the rank of other URLs."""
    num_urls = len(urls)
    for url in urls:
        yield (url, rank / num_urls)

if __name__ == "__main__":
    argc=len(sys.argv)
    if argc < 2 or argc > 8:
        print("Usage: pagerank <file-or-folder> [damp-factor=0.8] [iterations=100] [epsilon=1e-6] [parallel-factor=2] [break-lineage=20] [output-file]", file=sys.stderr)
        print("\tIf <file> is given, load a single file. If <folder> is given, load all 'part-*' files of that folder", file=sys.stderr)
        exit(-1)
    infile=sys.argv[1]
    damp=float(sys.argv[2]) if argc > 2 else 0.8
    damp_c=1-damp
    max_iteration=int(sys.argv[3]) if argc > 3 else 100
    epsilon=float(sys.argv[4]) if argc > 4 else 1e-6
    parallel_factor=int(sys.argv[5]) if argc > 5 else 2
    break_lineage=int(sys.argv[6]) if argc > 6 else 20
    outfile=sys.argv[7] if argc > 7 else ''

    # Initialize the spark context.
    sc = SparkContext()
    spark = SparkSession\
        .builder\
        .appName("PythonPageRank")\
        .getOrCreate()

    # load
    print(time.strftime('%H:%M:%S'), 'Loading')
    time0=time.time()
    # format:
    # a\ta b c d 
    # b\td e f 
    # c\t a d
    # ...
    lines = loadFile(infile, 'part-')
    #graph = lines.map(lambda urls: parseNeighbors(urls)).distinct().groupByKey().cache()
    graph = lines.map(lambda urls: parseNeighborList(urls)).cache()
    del lines

    n = graph.count()
    npart = max(graph.getNumPartitions(), sc.defaultParallelism)
    if graph.getNumPartitions() < sc.defaultParallelism:
        graph = graph.repartition(npart)
    maxnpart = parallel_factor*npart

    # initialize ranks
    print(time.strftime('%H:%M:%S'), 'Initializing')
    time1=time.time()
    ranks = graph.map(lambda url_neighbors: (url_neighbors[0], 1.0))
    progress = graph.count()*1.0
    
    # Calculates and updates URL ranks continuously using PageRank algorithm.
    print(time.strftime('%H:%M:%S'), 'Calculating')
    time2=time.time()   
    for iteration in range(max_iteration):
        time_iter=time.time()
        # Calculates URL contributions to the rank of other URLs.
        contribs = graph.join(ranks).flatMap(
            lambda url_urls_rank: computeContribs(url_urls_rank[1][0], url_urls_rank[1][1]))
        # Re-calculates URL ranks based on neighbor contributions.
        ranks = contribs.reduceByKey(add).mapValues(lambda rank: rank * damp + damp_c)  
        if ranks.getNumPartitions() >= maxnpart:
            ranks = ranks.coalesce(npart)
        if break_lineage != 0 and iteration != 0 and iteration % break_lineage == 0:
            #ranks = sc.parallelize(ranks_new.collect())
            ranks = ranks.cache()
            ranks.localCheckpoint()
        progress_new = ranks.map(lambda v: v[1]**2).sum()
        diff = abs(progress_new - progress)
        progress = progress_new
        time_iter = time.time()-time_iter
        print("finish iteration: %d, progress: %f, improvement: %f, used time: %f" % (iteration, progress, diff, time_iter))
        if diff <= epsilon:
            print('no more obvious improvement')
            break
    
    # Collects result
    print(time.strftime('%H:%M:%S'), 'Dumping')
    time3=time.time()
    if len(outfile) == 0:
        print("Skip outputting")
    elif outfile == '#console':
        for (link, rank) in ranks.collect():
            print("%d\t%f" % (link, rank))
    else:
        if os.path.exists(outfile):
            if os.path.isfile(outfile):
                os.remove(outfile)
            else:
                os.rmdir(outfile)
        ranks.saveAsTextFile(outfile)
        
    time4=time.time()
    
    print('iterations: %d' % min(iteration+1, max_iteration))
    print('loading time: %f' % (time1-time0))
    print('initialize time: %f' % (time2-time1))
    print('computing time: %f' % (time3-time2))
    print('dumping time: %f' % (time4-time3))
    
    spark.stop()
