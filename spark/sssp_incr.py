"""
Single Source Shortest Path

Example Usage:
bin/spark-submit sssp_incr.py data/graph.txt data/delta.txt data/ref.txt 0
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
        #n = 1
    else:
        fl = listFiles(infile, prefix)
        if len(fl) == 0 and opt_prefix is not None:
            fl = listFiles(infile, opt_prefix)
        tmp = [spark.read.text(infile+'/'+f).rdd.map(lambda r: r[0]) for f in fl]
        lines = sc.union(tmp)
        #n = len(fl)
    return lines
    #return lines, n

def parseItem(item):
    item=item.split(',')
    return int(item[0]), float(item[1])

def parseNeighborList(line):
    t=line.split('\t')
    key=int(t[0])
    value=[parseItem(v) for v in t[1].split(' ') if len(v)>0]
    return (key, value)

def parseRef(line):
    t=line.split('\t')
    key=int(t[0])
    value=float(t[1])
    return (key, value)
    
def parseDelta(line):
    t=line[0]
    s,d,v=line[2:].split(',')
    s=int(s)
    d=int(d)
    v=float(v)
    return (s,t,d,v)
    
def mergeDelta(record):
    # src, (links, deltas)
    s=record[0]
    l=record[1][0].data[0]
    if record[1][1].maxindex != 0:
        ms=record[1][1].data[0].data
        for m in ms:
            if m[1] == 'A':
                l.append((m[2]),m[3])
            elif m[1] == 'R':
                for i in range(len(l)):
                    if l[i][0] == m[2]:
                        del l[i]
                        break
            else:
                for i in range(len(l)):
                    if l[i][0] == m[2]:
                        #l[i][1] = m[3]
                        l[i] = (m[2], m[3])
                        break
    return (s,l)

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
    if argc < 4 or argc > 9:
        print("Usage: sssp-incr <graph-file> <delta-file> <ref-file> [source=0] [epsilon=1e-9] [parallel-factor=2] [break-lineage=20] [output-file]", file=sys.stderr)
        print("\tIf <*-file> is a file, load that file. If it is a directory, load all 'part-*', 'delta-', 'ref-' files of that directory", file=sys.stderr)
        exit(-1)
    infile=sys.argv[1]
    deltafile=sys.argv[2]
    reffile=sys.argv[3]
    source=int(sys.argv[4]) if argc > 4 else 0
    epsilon=float(sys.argv[5]) if argc > 5 else 1e-9
    parallel_factor=int(sys.argv[6]) if argc > 6 else 2
    break_lineage=int(sys.argv[7]) if argc > 7 else 20
    outfile=sys.argv[8] if argc > 8 else ''

    # Initialize the spark context.
    sc = SparkContext()
    spark = SparkSession\
        .builder\
        .appName("PythonSSSPIncr")\
        .getOrCreate()

    #nfiles = 1
    
    # load
    print(time.strftime('%H:%M:%S'), 'Loading')
    time0=time.time()
    # my format:
    # a\ta,x b,y c,z d,w 
    # b\td,u e,v f,o 
    # c\t a,p d,q
    # ...
    lines = loadFile(infile, 'part-')
    graph = lines.map(lambda l: parseNeighborList(l)) #.cache()
    graph = graph.map(lambda v: v if v[0] != source else modify_source(v, source))
    
    # delta file
    # format:
    # A 3,5,1.23
    # R 1,4,1.53
    # I 2,4,2.43
    # D 3,2,0.42
    lines = loadFile(deltafile, 'delta-')
    delta = lines.map(lambda l: parseDelta(l)).groupBy(lambda r: r[0])
    
    # change graph
    print(time.strftime('%H:%M:%S'), 'Changing graph')
    time01=time.time()
    graph = graph.cogroup(delta).map(mergeDelta)
    graph.cache()
    graph.localCheckpoint()
    n = graph.count()
    del delta
    
    npart = max(graph.getNumPartitions(), sc.defaultParallelism)
    if graph.getNumPartitions() < sc.defaultParallelism:
        graph = graph.repartition(npart)
    maxnpart = parallel_factor*npart
    
    # initialize sssp
    print(time.strftime('%H:%M:%S'), 'Initializing value')
    time1=time.time()
    lines = loadFile(reffile, 'ref-', 'value-')
    sssp = lines.map(parseRef)
    if sssp.getNumPartitions() < sc.defaultParallelism:
        sssp = sssp.repartition(npart)
    progress = sssp.map(lambda v: (0,1) if math.isinf(v[1]) else (v[1],0) ).reduce(progress_add)
    del lines
    
    # Calculates and updates sssp continuously
    print(time.strftime('%H:%M:%S'), 'Calculating')
    time2=time.time()
    for iteration in range(n):
        time_iter=time.time()
        contribs = graph.join(sssp).flatMap(
            lambda k_list_sp: computeContribs(k_list_sp[1][0], k_list_sp[1][1]))
        # Re-calculates sssp based on neighbor contributions.
        sssp = contribs.reduceByKey(min)
        # truncate the long lineage which greately slown down the process
        if sssp.getNumPartitions() >= maxnpart:
            sssp = sssp.coalesce(npart)
        if break_lineage != 0 and iteration != 0 and iteration % break_lineage == 0:
            #sssp = sc.parallelize(sssp.collect())
            sssp = sssp.cache()
            sssp.localCheckpoint()
        #progress_new = sssp.map(lambda v: (0,1) if math.isinf(v[1]) else (v[1],0) ).reduce(progress_add)
        progress_new = sssp.aggregate((0,0), (lambda lr,v:(lr[0],lr[1]+1) if math.isinf(v[1]) else(lr[0]+v[1],lr[1])), progress_add)
        #diff = sssp_new.join(sssp).mapValues(lambda p:abs(p[0]-p[1])).map(lambda pv: pv[1]).reduce(add)
        diff = (progress_new[0] - progress[0], progress_new[1] - progress[1])
        progress = progress_new
        time_iter = time.time()-time_iter
        print("finish iteration: %d, progress: (%f , %d), improvement: (%f , %d), used time: %f"
            % (iteration, progress[0], progress[1], diff[0], diff[1], time_iter))
        if diff[1] == 0 and abs(diff[0]) <= epsilon:
            print('no more obvious improvement')
            break
    
    time3=time.time()
    # Collects all URL sssp and dump them
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
    print('loading time: %f' % (time01-time0))
    print('changing time: %f' % (time1-time01))
    print('initialize time: %f' % (time2-time1))
    print('computing time: %f' % (time3-time2))
    print('dumping time: %f' % (time4-time3))
    
    spark.stop()
