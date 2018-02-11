"""
Single Source Shortest Path

Example Usage:
bin/spark-submit sssp.py data/xxx.txt 0
"""
from __future__ import print_function

import re
import sys
from operator import add
import time
import math

from pyspark.sql import SparkSession
from pyspark import SparkContext

def computeContribs(neighbors, sp):
    """Calculates URL contributions to the rank of other URLs."""
    for (dst, weight) in neighbors:
        yield (dst, weight + sp)

def parseItem(item):
    item=item.split(',')
    return int(item[0]), float(item[1])

def parseNeighborList(line):
    t=line.split('\t')
    key=int(t[0])
    value=[parseItem(v) for v in t[1].split(' ') if len(v)>0]
    return (key, value)

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
    if argc < 2 or argc > 6:
        print("Usage: sssp <file> [source=0] [epsilon=1e-6] [break-lineage=3] [output-file]", file=sys.stderr)
        exit(-1)
    infile=sys.argv[1]
    source=int(sys.argv[2]) if argc > 2 else 0
    epsilon=float(sys.argv[3]) if argc > 3 else 1e-6
    break_lineage=int(sys.argv[4]) if argc > 4 else 3
    outfile=sys.argv[5] if argc > 5 else ''

    # Initialize the spark context.
    sc = SparkContext()
    spark = SparkSession\
        .builder\
        .appName("PythonSSSP")\
        .getOrCreate()

    time0=time.time()
    # my format:
    # a\ta,x b,y c,z d,w 
    # b\td,u e,v f,o 
    # c\t a,p d,q
    # ...
    lines = spark.read.text(infile).rdd.map(lambda r: r[0])

    # Loads all nodes from input file and initialize their neighbors
    nodes = lines.map(lambda l: parseNeighborList(l)) #.cache()
    nodes = nodes.map(lambda v: v if v[0] != source else modify_source(v, source)).cache()
    n = nodes.count()

    # initialize sssp to be inf execpt the source (to be 0)
    sssp = nodes.map(lambda neighbors: (neighbors[0], math.inf if neighbors[0] != source else 0 ))
    progress = (0, n-1)
    
    time1=time.time()   
    # Calculates and updates sssp continuously
    for iteration in range(n):
        time_iter=time.time()
        contribs = nodes.join(sssp).flatMap(
            lambda k_list_sp: computeContribs(k_list_sp[1][0], k_list_sp[1][1]))
        # Re-calculates sssp based on neighbor contributions.
        #sssp_new = sssp.join(contribs.reduceByKey(min)).mapValues(min).cache() # without adding (source, 0) to source's neighbor list
        sssp_new = contribs.reduceByKey(min).cache()
        progress_new = sssp_new.map(lambda v: (0,1) if math.isinf(v[1]) else (v[1],0) ).reduce(progress_add)
        #diff = sssp_new.join(sssp).mapValues(lambda p:abs(p[0]-p[1])).map(lambda pv: pv[1]).reduce(add)
        diff = (progress_new[0] - progress[0], progress_new[1] - progress[1])
        # to cut down the long linage which greately slown down the process
        if break_lineage != 0 and iteration != 0 and iteration % break_lineage == 0:
            sssp = sc.parallelize(sssp_new.collect())
        else:
            sssp = sssp_new
        progress = progress_new
        time_iter = time.time()-time_iter
        print("finish iteration: %d, progress: (%f , %d), improvement: (%f , %d), used (second): %f"
			% (iteration, progress[0], progress[1], diff[0], diff[1], time_iter))
        if diff[1] == 0 and abs(diff[0]) < epsilon:
            break
    
    time2=time.time()
    # Collects all URL sssp and dump them
    if len(outfile) == 0:
        print("Skip outputting")
    elif outfile == '#console':
        for (link, rank) in sssp.collect():
            print("%d\t%f" % (link, rank))
    else:
        sssp.saveAsTextFile(outfile)
        
    time3=time.time()
    
    print('iterations: %d' % (iteration+1))
    print('loading time: %f' % (time1-time0))
    print('computing time: %f' % (time2-time1))
    print('dumping time: %f' % (time3-time2))
    
    spark.stop()
