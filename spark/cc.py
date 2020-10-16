"""
Connected Component

Example Usage:
bin/spark-submit cc.py data/graph.txt 0
or:
bin/spark-submit cc.py data/graph.txt 0 data/delta.txt data/ref.txt
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

if __name__ == "__main__":
    argc=len(sys.argv)
    if argc < 2 or argc > 7:
        print("Usage: cc <graph-file> [delta-file=-] [ref-file=-] [parallel-factor=2] [break-lineage=20] [output-file]", file=sys.stderr)
        print("\tIf <*-file> is a file, load that file. If it is a directory, load all 'part-*', 'delta-', 'ref-' files of that directory", file=sys.stderr)
        print("\tIf <delta> and <ref> are given, run the incremental version.")
        exit(-1)
    infile=sys.argv[1]
    deltafile=sys.argv[2] if argc > 2 else ''
    reffile=sys.argv[3] if argc > 3 else ''
    parallel_factor=int(sys.argv[4]) if argc > 4 else 2
    break_lineage=int(sys.argv[5]) if argc > 5 else 20
    outfile=sys.argv[6] if argc > 6 else ''

    if len(deltafile) > 0 and deltafile != '-' and len(reffile) > 0 and reffile != '-':
        do_incremental = True
    else:
        do_incremental = False

    # Initialize the spark context.
    sc = SparkContext()
    spark = SparkSession\
        .builder\
        .appName("PythonConnectedComponent")\
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
    graph.cache()
    n = graph.count()
    
    if do_incremental:
        del delta
    
    npart = max(graph.getNumPartitions(), sc.defaultParallelism)
    if graph.getNumPartitions() < sc.defaultParallelism:
        graph = graph.repartition(npart)
    maxnpart = parallel_factor*npart
    
    # initialize cc
    print(time.strftime('%H:%M:%S'), 'Initializing value')
    time1=time.time()
    if do_incremental:
        lines = loadFile(reffile, 'ref-', 'value-')
        cc = lines.map(parseRef)
        if cc.getNumPartitions() < sc.defaultParallelism:
            cc = cc.repartition(npart)
    else:
        cc = graph.map(lambda neighbors: (neighbors[0], neighbors[0]))
    progress = cc.aggregate(0, (lambda lr,v:lr+v[1]), add)
    del lines
    
    # Calculates and updates cc continuously
    print(time.strftime('%H:%M:%S'), 'Calculating')
    time2=time.time()   
    for iteration in range(n):
        time_iter=time.time()
        contribs = graph.join(cc).flatMap(
            lambda k_list_sp: computeContribs(k_list_sp[1][0], k_list_sp[1][1]))
        cc = contribs.reduceByKey(min).cache()
        # Re-calculates cc based on neighbor contributions.
        if cc.getNumPartitions() >= maxnpart:
            cc = cc.coalesce(npart)
        if break_lineage != 0 and iteration != 0 and iteration % break_lineage == 0:
            #cc = sc.parallelize(cc.collect())
            cc = cc.cache()
            cc.localCheckpoint()
        progress_new = cc.aggregate(0, (lambda lr,v:lr+v[1]), add)
        diff = progress_new - progress
        progress = progress_new
        time_iter = time.time()-time_iter
        print("finish iteration: %d, progress: %d, improvement: %d, used (second): %f" % (iteration, progress, diff, time_iter))
        if diff == 0:
            print('no more obvious improvement')
            break
    
    time3=time.time()
    # Collects all URL cc and dump them
    if len(outfile) == 0:
        print("Skip outputting")
    else:
        if cc.count() != n:
            cc = graph.leftOuterJoin(cc).map(lambda klv:klv[1][1] if klv[1][1] is not None else klv[0])
        if outfile == '#console':
            for (link, rank) in cc.collect():
                print("%d\t%f" % (link, rank))
        else:
            if os.path.exists(outfile):
                if os.path.isfile(outfile):
                    os.remove(outfile)
                else:
                    os.rmdir(outfile)
            cc.saveAsTextFile(outfile)
        
    time4=time.time()
    
    print('iterations: %d' % (iteration+1))
    print('loading time: %f' % ((time01 if do_incremental else time1)-time0))
    if do_incremental:
        print('changing time: %f' % (time1-time01))
    print('initialize time: %f' % (time2-time1))
    print('computing time: %f' % (time3-time2))
    print('dumping time: %f' % (time4-time3))
    
    spark.stop()
