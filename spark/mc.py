"""
Markov Chain

Weighted & Directed

Example Usage:
bin/spark-submit mc.py data/xxx.txt
"""
from __future__ import print_function

import re
import sys
from operator import add
import time
import math
import random

from pyspark.sql import SparkSession
from pyspark import SparkContext


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

def progress_fun(v):
    return v[1]**2

def normalize(values):
    s=values.map(lambda r: r[1]).reduce(add)
    if s != 1.0:
        values=values.map(lambda r: (r[0], r[1]/s))
    return values

if __name__ == "__main__":
    argc=len(sys.argv)
    if argc < 2 or argc > 6:
        print("Usage: mc <file> [epsilon=1e-6] [break-lineage=3] [output-file]", file=sys.stderr)
        exit(-1)
    infile=sys.argv[1]
    epsilon=float(sys.argv[2]) if argc > 2 else 1e-6
    break_lineage=int(sys.argv[3]) if argc > 3 else 3
    outfile=sys.argv[4] if argc > 4 else ''

    # Initialize the spark context.
    sc = SparkContext()
    spark = SparkSession\
        .builder\
        .appName("PythonMC")\
        .getOrCreate()

    time0=time.time()
    # my format:
    # a\ta,x b,y c,z d,w 
    # b\td,u e,v f,o 
    # c\t a,p d,q
    # ...
    lines = spark.read.text(infile).rdd.map(lambda r: r[0])

    # Loads all nodes from input file and initialize their neighbors
    nodes = lines.map(lambda l: parseNeighborListNormalize(l)).cache()
    n = nodes.count()

    # initialize mc to be a random distribution
    mc = nodes.map(lambda neighbors: (neighbors[0], random.random())).cache()
    # this cache is VERY IMPORTANT, otherwise mc will be a new random number each time it is referred
    mc = normalize(mc)
    progress = mc.map(progress_fun).reduce(add)
    
    time1=time.time()   
    # Calculates and updates mc continuously
    for iteration in range(n):
        time_iter=time.time()
        contribs = nodes.join(mc).flatMap(
            lambda k_list_sp: computeContribs(k_list_sp[1][0], k_list_sp[1][1]))
        # Re-calculates mc based on neighbor contributions.
        mc_new = contribs.reduceByKey(add).cache()
        progress_new = mc_new.map(progress_fun).reduce(add)
        #diff = mc_new.join(mc).mapValues(lambda p:abs(p[0]-p[1])).map(lambda pv: pv[1]).reduce(add)
        diff = progress_new - progress
        # to cut down the long linage which greately slown down the process
        if break_lineage != 0 and iteration != 0 and iteration % break_lineage == 0:
            mc = sc.parallelize(mc_new.collect())
        else:
            mc = mc_new
        progress = progress_new
        time_iter = time.time()-time_iter
        print("finish iteration: %d, progress: %f, improvement: %f, used (second): %f"
            % (iteration, progress, diff, time_iter))
        if abs(diff) < epsilon:
            break
    
    time2=time.time()
    # Collects all URL mc and dump them
    if len(outfile) == 0:
        print("Skip outputting")
    elif outfile == '#console':
        for (link, res) in mc.collect():
            print("%d\t%f" % (link, res))
    else:
        mc.saveAsTextFile(outfile)
        
    time3=time.time()
    
    print('iterations: %d' % (iteration+1))
    print('loading time: %f' % (time1-time0))
    print('computing time: %f' % (time2-time1))
    print('dumping time: %f' % (time3-time2))
    
    spark.stop()
