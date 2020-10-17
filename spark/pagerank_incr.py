"""
This is an example implementation of PageRank for incremental case.

Example Usage:
bin/spark-submit pagerank_incr.py data/pagerank_data.txt data/delta_data.txt 10 1e-8
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
    
def parseRef(line):
    t=line.split('\t')
    key=int(t[0])
    value=float(t[1])
    return (key, value)

def parseDelta(line):
    t=line[0]
    s,d=line[2:].split(',')
    s=int(s)
    d=int(d)
    return (s,t,d)
    
def mergeDelta(record):
    # src, (links, deltas)
    s=record[0]
    l=record[1][0].data[0]
    if record[1][1].maxindex != 0:
        ms=record[1][1].data[0].data
        for m in ms:
            if m[1] == 'A':
                l.append(m[2])
            else: # m[1] == 'R':
                l.remove(m[2])
    return (s,l)

def computeContribs(urls, rank):
    """Calculates URL contributions to the rank of other URLs."""
    num_urls = len(urls)
    for url in urls:
        yield (url, rank / num_urls)

if __name__ == "__main__":
    argc=len(sys.argv)
    if argc < 4 or argc > 10:
        print("Usage: pagerank-incr <graph-file> <delta-file> <ref-file> [damp-factor=0.8] [iterations=100] [epsilon=1e-6] [parallel-factor=2] [break-lineage=20] [output-file]", file=sys.stderr)
        print("\tIf <*-file> is a file, load that file. If it is a directory, load all 'part-*', 'delta-', 'ref-' files of that directory", file=sys.stderr)
        exit(-1)
    infile=sys.argv[1]
    deltafile=sys.argv[2]
    reffile=sys.argv[3]
    damp=float(sys.argv[4]) if argc > 4 else 0.8
    damp_c=1-damp
    max_iteration=int(sys.argv[5]) if argc > 5 else 100
    epsilon=float(sys.argv[6]) if argc > 6 else 1e-6
    parallel_factor=int(sys.argv[7]) if argc > 7 else 2
    break_lineage=int(sys.argv[8]) if argc > 8 else 20
    outfile=sys.argv[9] if argc > 9 else ''

    # Initialize the spark context.
    sc = SparkContext()
    spark = SparkSession\
        .builder\
        .appName("PythonPageRankIncr")\
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
    graph = lines.map(lambda urls: parseNeighborList(urls))

    # delta file
    # format:
    # A 3,5
    # R 1,4
    lines = loadFile(deltafile, 'delta-')
    delta = lines.map(lambda l: parseDelta(l)).groupBy(lambda r: r[0])
    
    # change graph
    print(time.strftime('%H:%M:%S'), 'Changing graph')
    time01=time.time()
    graph = graph.cogroup(delta).map(mergeDelta).cache()
    del delta
    
    n = graph.count()
    npart = max(graph.getNumPartitions(), sc.defaultParallelism)
    if graph.getNumPartitions() < sc.defaultParallelism:
        graph = graph.repartition(npart)
    maxnpart = parallel_factor*npart

    # initialize ranks
    print(time.strftime('%H:%M:%S'), 'Initializing value')
    time1=time.time()
    lines = loadFile(reffile, 'ref-', 'value-')
    ranks = lines.map(parseRef)
    if ranks.getNumPartitions() < sc.defaultParallelism:
        ranks = ranks.repartition(npart)
    progress = ranks.map(lambda v: v[1]**2).sum()
    del lines
    
    # Calculates and updates URL ranks continuously using PageRank algorithm.
    print(time.strftime('%H:%M:%S'), 'Calculating')
    time2=time.time()
    for iteration in range(max_iteration):
        time_iter=time.time()
        # Calculates URL contributions to the rank of other URLs.
        contribs = graph.join(ranks).flatMap(
            lambda url_urls_rank: computeContribs(url_urls_rank[1][0], url_urls_rank[1][1]))
        # Re-calculates URL ranks based on neighbor contributions.
        ranks = contribs.reduceByKey(add).mapValues(lambda r: r * damp + damp_c)        
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
    print('loading time: %f' % (time01-time0))
    print('changing time: %f' % (time1-time01))
    print('initialize time: %f' % (time2-time1))
    print('computing time: %f' % (time3-time2))
    print('dumping time: %f' % (time4-time3))
    
    spark.stop()
