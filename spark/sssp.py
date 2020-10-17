"""
Single Source Shortest Path

Example Usage:
bin/spark-submit sssp.py data/xxx.txt 0
"""
from __future__ import print_function

import re
import os
import sys
from operator import add
import time
import math

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

def parseItem(item):
    item=item.split(',')
    return int(item[0]), float(item[1])

def parseNeighborList(line):
    t=line.split('\t')
    key=int(t[0])
    value=[parseItem(v) for v in t[1].split(' ') if len(v)>0]
    return (key, value)

def computeContribs(neighbors, sp):
    for (dst, weight) in neighbors:
        yield (dst, weight + sp)

def modify_source(source_node, source_id):
    id = source_node[0]
    neighbors = source_node[1]
    if (source_id, 0) not in neighbors:
        neighbors.append((source_id, 0))
    return id, neighbors

def progress_add(a, b):
    return a[0]+b[0], a[1]+b[1]

if __name__ == "__main__":
    argc=len(sys.argv)
    if argc < 2 or argc > 7:
        print("Usage: sssp <file-or-folder> [source=0] [epsilon=1e-9] [parallel-factor=2] [break-lineage=20] [output-file]", file=sys.stderr)
        print("\tIf <file> is given, load a single file. If <folder> is given, load all 'part-*' files of that folder", file=sys.stderr)
        exit(-1)
    infile=sys.argv[1]
    source=int(sys.argv[2]) if argc > 2 else 0
    epsilon=float(sys.argv[3]) if argc > 3 else 1e-9
    parallel_factor=int(sys.argv[4]) if argc > 4 else 2
    break_lineage=int(sys.argv[5]) if argc > 5 else 20
    outfile=sys.argv[6] if argc > 6 else ''

    # Initialize the spark context.
    sc = SparkContext()
    spark = SparkSession\
        .builder\
        .appName("PythonSSSP")\
        .getOrCreate()

    # load
    print(time.strftime('%H:%M:%S'), 'Loading')
    time0=time.time()
    # my format:
    # a\ta,w1 b,w2 c,w3 d,w4 
    # b\td,w1 e,w1 f,w3 
    # c\ta,w1 d,w2
    # ...
    lines = loadFile(infile, 'part-')
    #graph = lines.map(lambda urls: parseNeighbors(urls)).distinct().groupByKey().cache()
    graph = lines.map(lambda urls: parseNeighborList(urls))
    graph = graph.map(lambda v: v if v[0] != source else modify_source(v, source)).cache()
    del lines

    n = graph.count()
    npart = max(graph.getNumPartitions(), sc.defaultParallelism)
    if graph.getNumPartitions() < sc.defaultParallelism:
        graph = graph.repartition(nparallel)
    maxnpart = parallel_factor*npart

    # initialize sssp to be inf execpt the source (to be 0)
    print(time.strftime('%H:%M:%S'), 'Initializing')
    time1=time.time()
    sssp = graph.map(lambda kl: (kl[0], math.inf if kl[0] != source else 0 ))
    progress = (0, n-1)
    
    # Calculates and updates sssp continuously
    print(time.strftime('%H:%M:%S'), 'Calculating')
    time2=time.time()
    for iteration in range(n):
        time_iter=time.time()
        contribs = graph.join(sssp).flatMap(
            lambda k_list_sp: computeContribs(k_list_sp[1][0], k_list_sp[1][1]))
        # Re-calculates sssp based on neighbor contributions.
        #sssp_new = sssp.join(contribs.reduceByKey(min)).mapValues(min).cache() # without adding (source, 0) to source's neighbor list
        sssp = contribs.reduceByKey(min)
        # truncate the long lineage which greately slown down the process
        if sssp.getNumPartitions() >= maxnpart:
            sssp = sssp.coalesce(npart)
        if break_lineage != 0 and iteration != 0 and iteration % break_lineage == 0:
            #sssp = sc.parallelize(sssp.collect())
            sssp = sssp.cache()
            sssp.localCheckpoint()
        progress_new = sssp.map(lambda v: (0,1) if math.isinf(v[1]) else (v[1],0) ).reduce(progress_add)
        #diff = sssp_new.join(sssp).mapValues(lambda p:abs(p[0]-p[1])).map(lambda pv: pv[1]).reduce(add)
        diff = (progress_new[0] - progress[0], progress_new[1] - progress[1])
        progress = progress_new
        time_iter = time.time()-time_iter
        print("finish iteration: %d, progress: (%f , %d), improvement: (%f , %d), used time: %f"
            % (iteration, progress[0], progress[1], diff[0], diff[1], time_iter))
        if diff[1] == 0 and abs(diff[0]) <= epsilon:
            print('no more obvious improvement')
            break
    
    # Collects result
    print(time.strftime('%H:%M:%S'), 'Dumping')
    time3=time.time()
    if len(outfile) == 0:
        print("Skip outputting")
    else:
        if sssp.count() != n:
            sssp = graph.leftOuterJoin(sssp).mapValues(lambda lv:lv[1] if lv[1] is not None else math.inf)
        if outfile == '#console':
            for (link, rank) in sssp.collect():
                print("%d\t%f" % (link, rank))
        else:
            if os.path.exists(outfile):
                if os.path.isfile(outfile):
                    os.remove(outfile)
                else:
                    os.rmdir(outfile)
            sssp.saveAsTextFile(outfile)
        
    time4=time.time()
    
    print('iterations: %d' % (iteration+1))
    print('loading time: %f' % (time1-time0))
    print('initialize time: %f' % (time2-time1))
    print('computing time: %f' % (time3-time2))
    print('dumping time: %f' % (time4-time3))
    
    spark.stop()
