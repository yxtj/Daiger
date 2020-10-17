"""
Markov Chain

Weighted & Directed

Example Usage:
bin/spark-submit mc.py data/xxx.txt
"""
from __future__ import print_function

import re
import os
import sys
from operator import add
import time
import math
import random

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

def computeContribs(neighbors, prob):
    """Calculates URL contributions to the rank of other URLs."""
    for (dst, weight) in neighbors:
        yield (dst, weight * prob)

def parseItem(item):
    item=item.split(',')
    return int(item[0]), float(item[1])

def parseNeighborList(line):
    t=line.split('\t')
    key=int(t[0])
    value=[parseItem(v) for v in t[1].split(' ') if len(v)>0]
    return (key, value)

def parseNeighborListNormalize(line):
    t=line.split('\t')
    key=int(t[0])
    value=[parseItem(v) for v in t[1].split(' ') if len(v)>0]
    s=sum(v[1] for v in value)
    res=[]
    for n,w in value:
        res.append((n, w/s))
    return (key, res)

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

def normalizeEdge(links):
    s = sum(w for d,w in links)
    return [(d,w/s) for d,w in links]

def normalize(values):
    s=values.map(lambda r: r[1]).sum()
    if s != 1.0:
        values=values.map(lambda r: (r[0], r[1]/s))
    return values

if __name__ == "__main__":
    argc=len(sys.argv)
    if argc < 2 or argc > 7:
        print("Usage: mc <graph-file> [delta-file=-] [ref-file=-] [epsilon=1e-6] [parallel-factor=2] [break-lineage=20] [output-file]", file=sys.stderr)
        print("\tIf <*-file> is a file, load that file. If it is a directory, load all 'part-*', 'delta-', 'ref-' files of that directory", file=sys.stderr)
        print("\tIf <delta> and <ref> are given, run the incremental version.")
        exit(-1)
    infile=sys.argv[1]
    deltafile=sys.argv[2] if argc > 2 else ''
    reffile=sys.argv[3] if argc > 3 else ''
    epsilon=float(sys.argv[4]) if argc > 4 else 1e-6
    parallel_factor=int(sys.argv[5]) if argc > 5 else 2
    break_lineage=int(sys.argv[6]) if argc > 6 else 20
    outfile=sys.argv[7] if argc > 7 else ''

    if len(deltafile) > 0 and deltafile != '-' and len(reffile) > 0 and reffile != '-':
        do_incremental = True
    else:
        do_incremental = False

    # Initialize the spark context.
    sc = SparkContext()
    spark = SparkSession\
        .builder\
        .appName("PythonMC")\
        .getOrCreate()

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
    
    graph = graph.mapValues(normalizeEdge)
    graph.cache()
    n = graph.count()
    
    if do_incremental:
        del delta
    
    npart = max(graph.getNumPartitions(), sc.defaultParallelism)
    if npart % sc.defaultParallelism != 0:
        npart += sc.defaultParallelism - (npart % sc.defaultParallelism)
    if graph.getNumPartitions() < sc.defaultParallelism:
        graph = graph.repartition(npart)
    maxnpart = parallel_factor*npart
    
    # initialize mc
    print(time.strftime('%H:%M:%S'), 'Initializing value')
    time1=time.time()
    if do_incremental:
        lines = loadFile(reffile, 'ref-', 'value-')
        mc = lines.map(parseRef)
        if mc.getNumPartitions() < sc.defaultParallelism:
            mc = mc.repartition(npart)
    else:
        mc = graph.map(lambda node: (node[0], 1.0/n))
    progress = mc.aggregate(0.0, (lambda lr,v:lr+v[1]**2), add)
    del lines

    # Calculates and updates mc continuously
    print(time.strftime('%H:%M:%S'), 'Calculating')
    time2=time.time()   
    for iteration in range(n):
        time_iter=time.time()
        contribs = graph.join(mc).flatMap(
            lambda k_list_sp: computeContribs(k_list_sp[1][0], k_list_sp[1][1]))
        # Re-calculates mc based on neighbor contributions.
        mc = contribs.reduceByKey(add)
        if mc.getNumPartitions() >= maxnpart:
            mc = mc.coalesce(npart)
        # truncate the long lineage which greately slown down the process
        if break_lineage != 0 and iteration != 0 and iteration % break_lineage == 0:
            #mc = sc.parallelize(mc.collect())
            mc = mc.cache()
            mc.localCheckpoint()
        progress_new = mc.aggregate(0.0, (lambda lr,v:lr+v[1]**2), add)
        diff = progress_new - progress
        progress = progress_new
        time_iter = time.time()-time_iter
        print("finish iteration: %d, progress: %f, improvement: %f, used time: %f" % (iteration, progress, diff, time_iter))
        if abs(diff) <= epsilon:
            break
    
    time3=time.time()
    # Collects all URL mc and dump them
    if len(outfile) == 0:
        print("Skip outputting")
    else:
        if mc.count() != n:
            mc = graph.leftOuterJoin(mc).mapValues(lambda lv:lv[1][1] if lv[1][1] is not None else 0)
        if outfile == '#console':
            for (link, rank) in mc.collect():
                print("%d\t%f" % (link, rank))
        else:
            if os.path.exists(outfile):
                if os.path.isfile(outfile):
                    os.remove(outfile)
                else:
                    os.rmdir(outfile)
            mc.saveAsTextFile(outfile)
        
    time4=time.time()
    
    print('iterations: %d' % (iteration+1))
    print('loading time: %f' % ((time01 if do_incremental else time1)-time0))
    if do_incremental:
        print('changing time: %f' % (time1-time01))
    print('initialize time: %f' % (time2-time1))
    print('computing time: %f' % (time3-time2))
    print('dumping time: %f' % (time4-time3))
    
    spark.stop()
