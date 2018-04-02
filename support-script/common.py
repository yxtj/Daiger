import os, sys, re

# file name
def get_file_names(folder, prefix):
    l=os.listdir(folder);
    gpat=re.compile('^'+prefix+'\\d+$')
    gfiles=[]
    for fn in l:
        if gpat.match(fn):
            gfiles.append(fn)
    gfiles.sort()
    return gfiles

def gen_file_names(folder, prefix, n):
    gfiles=[]
    for i in range(n):
        gfiles.append(folder+'/'+prefix+str(i))
    return gfiles

# load graph
def load_graph_weight(folder, prefix, n):
    if n == None:
        names=get_graph_file_names(folder, prefix);
        n = len(names)
    else:
        names=gen_graph_file_names(folder, prefix, n);
    g={}
    for fn in names:
        with open(fn) as f:
            gdata=[l for l in f.read().split('\n') if len(l)!=0]
            for i in range(len(gdata)):
                key, line=gdata[i].split('\t')
                line = [l.split(',') for l in line.split(' ') if len(l)!=0]
                g[int(key)]=[(int(e),float(w)) for e,w in line]
    return (g, n)

def load_graph_unweight(folder, prefix, n):
    if n == None:
        names=get_graph_file_names(folder, prefix);
        n = len(names)
    else:
        names=gen_graph_file_names(folder, prefix, n);
    g={}
    for fn in names:
        with open(fn) as f:
            gdata=[l for l in f.read().split('\n') if len(l)!=0]
            for i in range(len(gdata)):
                key, line=gdata[i].split('\t')
                g[int(key)]=[int(v) for v in line.split(' ') if len(l)!=0]
    return (g,n)

# load result/value
def load_value(folder, prefix, n):
    if n == None:
        names=get_graph_file_names(folder, prefix);
    else:
        names=gen_graph_file_names(folder, prefix, n);
    value = {}
    for fn in names:
        with open(fn) as f:
            data=[l for l in f.read().split('\n') if len(l)!=0]
            for l in data:
                k, v = l.split('\t')
                value[int(k)] = float(v)
    return value

# load delta
def load_delta(folder, prefix, n):
    if n == None:
        names=get_graph_file_names(folder, prefix);
    else:
        names=gen_graph_file_names(folder, prefix, n);
    resA=[]
    resR=[]
    resI=[]
    resD=[]
    for fn in names:
        with open(fn) as f:
            data=[l for l in f.read().split('\n') if len(l)!=0]
            for l in data:
                for line in ddata:
                    #print(line)
                    tp = line[0]
                    line=line[2:].split(',')
                    f=int(line[0])
                    t=int(line[1])
                    if tp == 'A':
                        resA.append((f,t))
                    elif tp == 'R':
                        resR.append((f,t))
                    elif tp == 'I':
                        w=float(line[2])
                        resI.append((f,t,w))
                    else:
                        w=float(line[2])
                        resD.append((f,t,w))
    return (resA, resR, resI, resD)

# merge delta
def merge_delta_weight(g, d):
    # add
    for l in d[0]:
        g[l[0]].append(l[1])
    # remove
    for l in d[1]:
        f=l[0]
        t=l[1]
        length=len(g[f])
        idx=[i for i in range(length) if g[f][i][0]==t][0]
        del g[f][idx]
    # increase
    for l in d[2]:
        f=l[0]
        t=l[1]
        length=len(g[f])
        idx=[i for i in range(length) if g[f][i][0]==t][0]
        g[f][idx][1]=l[2]
    # decrease
    for l in d[2]:
        f=l[0]
        t=l[1]
        length=len(g[f])
        idx=[i for i in range(length) if g[f][i][0]==t][0]
        g[f][idx][1]=l[2]
    return g

def merge_delta_unweight(g, d):
    # add
    for l in d[0]:
        g[l[0]].append(l[1])
    # remove
    for l in d[1]:
        g[l[0]].remove(l[1])
    return g
    
# g is dict{key, list(<to, weight>)}
def dump_graph_weight(folder, prefix, n, g):
    names=gen_file_names(folder, prefix, n)
    flist=[]
    for n in names:
        flist.append(open(n, 'w'))
    for k,line in g.items():
        idx=k%n
        f=flist[idx]
        f.write(str(k)+'\t')
        for e,w in line:
            f.write('%d,%f ' % (e,w))
        f.write('\n')
    for f in flist:
        f.close()
    
# g is dict{key, list(to)}
def dump_graph_unweight(fn, g):
    names=gen_file_names(folder, prefix, n, g)
    flist=[]
    for n in names:
        flist.append(open(n, 'w'))
    for k,line in g.items():
        idx=k%n
        f=flist[idx]
        f.write(str(k)+'\t')
        for e in line:
            f.write(str(e) + ' ')
        f.write('\n')
    for f in flist:
        f.close()

