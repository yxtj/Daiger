"""
Single Source Widest Path

Example Usage:
bin/spark-submit sswp.py data/graph.txt 0
or:
bin/spark-submit sswp.py data/graph.txt 0 data/delta.txt data/ref.txt
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

def computeContribs(neighbors, wp):
    for (dst, weight) in neighbors:
        yield (dst, min(weight, wp))

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

def modify_source(source_node, source_id):
    id = source_node[0]
    neighbors = source_node[1]
    if (source_id, math.inf) not in neighbors:
        neighbors.append((source_id, math.inf))
    return id, neighbors

if __name__ == "__main__":
    argc=len(sys.argv)
    if argc < 2 or argc > 9:
        print("Usage: sswp-incr <graph-file> [source=0] [delta-file=-] [ref-file=-] [epsilon=1e-9] [parallel-factor=2] [break-lineage=20] [output-file]", file=sys.stderr)
        print("\tIf <*-file> is a file, load that file. If it is a directory, load all 'part-*', 'delta-', 'ref-' files of that directory", file=sys.stderr)
        print("\tIf <delta> and <ref> are given, run the incremental version.")
        exit(-1)
    infile=sys.argv[1]
    source=int(sys.argv[2]) if argc > 2 else 0
    deltafile=sys.argv[3] if argc > 3 else ''
    reffile=sys.argv[4] if argc > 4 else ''
    epsilon=float(sys.argv[5]) if argc > 5 else 1e-9
    parallel_factor=int(sys.argv[6]) if argc > 6 else 2
    break_lineage=int(sys.argv[7]) if argc > 7 else 20
    outfile=sys.argv[8] if argc > 8 else ''

    if len(deltafile) > 0 and deltafile != '-' and len(reffile) > 0 and reffile != '-':
        do_incremental = True
    else:
        do_incremental = False

    # Initialize the spark context.
    sc = SparkContext()
    spark = SparkSession\
        .builder\
        .appName("PythonSSWPIncr")\
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
    
    if do_incremental:
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
    
    if do_incremental:
        del delta
    
    npart = max(graph.getNumPartitions(), sc.defaultParallelism)
    if graph.getNumPartitions() < sc.defaultParallelism:
        graph = graph.repartition(npart)
    maxnpart = parallel_factor*npart
    
    # initialize sswp
    print(time.strftime('%H:%M:%S'), 'Initializing value')
    time1=time.time()
    if do_incremental:
        lines = loadFile(reffile, 'ref-', 'value-')
        sswp = lines.map(parseRef)
        if sswp.getNumPartitions() < sc.defaultParallelism:
            sswp = sswp.repartition(npart)
    else:
        sswp = graph.map(lambda v:(v[0], 0 if v[0] != source else math.inf))
    progress = sswp.aggregate(0, (lambda lr,v:lr+(v[1] if v[0] != source else 0)), add)
    del lines
    
    # Calculates and updates sswp continuously
    print(time.strftime('%H:%M:%S'), 'Calculating')
    time2=time.time()
    for iteration in range(n):
        time_iter=time.time()
        contribs = graph.join(sswp).flatMap(
            lambda k_list_wp: computeContribs(k_list_wp[1][0], k_list_wp[1][1]))
        # Re-calculates sswp based on neighbor contributions.
        sswp = contribs.reduceByKey(max)
        # truncate the long lineage which greately slown down the process
        if sswp.getNumPartitions() >= maxnpart:
            sswp = sswp.coalesce(npart)
        if break_lineage != 0 and iteration != 0 and iteration % break_lineage == 0:
            #sswp = sc.parallelize(sswp.collect())
            sswp = sswp.cache()
            sswp.localCheckpoint()
        progress_new = sswp.aggregate(0, (lambda lr,v:lr+(v[1] if v[0] != source else 0)), add)
        diff = progress_new - progress
        progress = progress_new
        time_iter = time.time()-time_iter
        print("finish iteration: %d, progress: %f, improvement: %f, used time: %f" % (iteration, progress, diff, time_iter))
        if diff <= epsilon:
            print('no more obvious improvement')
            break
    
    time3=time.time()
    # Collects all URL sswp and dump them
    if len(outfile) == 0:
        print("Skip outputting")
    else:
        if sswp.count() != n:
            sswp = graph.leftOuterJoin(sswp).mapValues(lambda v:v[1] if not math.isinf(v[1]) else math.inf)
        if outfile == '#console':
            for (link, rank) in sswp.collect():
                print("%d\t%f" % (link, rank))
        else:
            if os.path.exists(outfile):
                if os.path.isfile(outfile):
                    os.remove(outfile)
                else:
                    os.rmdir(outfile)
            sswp.saveAsTextFile(outfile)
        
    time4=time.time()
    
    print('iterations: %d' % (iteration+1))
    print('loading time: %f' % ((time01 if do_incremental else time1)-time0))
    if do_incremental:
        print('changing time: %f' % (time1-time01))
    print('initialize time: %f' % (time2-time1))
    print('computing time: %f' % (time3-time2))
    print('dumping time: %f' % (time4-time3))
    
    spark.stop()
