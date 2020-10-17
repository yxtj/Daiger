import re
import os
import math

def listFiles(fld, prefix):
    l = os.listdir(fld)
    l = [f for f in l if os.path.isfile(fld+'/'+f) and f.startswith(prefix)]
    ptn = prefix+'\\d+'
    l = [f for f in l if re.match(ptn, f)]
    return l

def loadFile(spark, sc, infile, prefix, opt_prefix=None):
    if os.path.isfile(infile):
        lines = spark.read.text(infile).rdd.map(lambda r: r[0])
        #n = 1
        return lines
    else:
        if isinstance(prefix, str):
            prefix = [prefix]
        for pf in prefix:
            fl = listFiles(infile, pf)
            if len(fl) == 0:
                continue
            tmp = [spark.read.text(infile+'/'+f).rdd.map(lambda r: r[0]) for f in fl]
            lines = sc.union(tmp)
            #n = len(fl)
            return lines
    return None

def parseDstWgtPair(item):
    item=item.split(',')
    return int(item[0]), float(item[1])

def parseNeighborListWeighted(line):
    t=line.split('\t')
    key=int(t[0])
    value=[parseDstWgtPair(v) for v in t[1].split(' ') if len(v)>0]
    return (key, value)

def parseNeighborListUnweighted(line):
    t=line.split('\t')
    key=int(t[0])
    value=[parseItem(v) for v in t[1].split(' ') if len(v)>0]
    return (key, value)

def parseRef(line):
    t=line.split('\t')
    key=int(t[0])
    value=float(t[1])
    return (key, value)
    
def parseDeltaWeighted(line):
    t=line[0]
    s,d,v=line[2:].split(',')
    s=int(s)
    d=int(d)
    v=float(v)
    return (s,t,d,v)

def parseDeltaUnweighted(line):
    t=line[0]
    s,d=line[2:].split(',')
    s=int(s)
    d=int(d)
    return (s,t,d)

def mergeDeltaWeighted(record):
    # src, (links, deltas)
    '''
    <record> is a result of a.cogroup(b)
    Each entry is a pair: (key, (links, deltas)). The second element is a pair of iterable .
    links[0] is a node's neighbor list.
    '''
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

def mergeDeltaUnweighted(record):
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

