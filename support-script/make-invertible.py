import sys
import numpy as np
import common

# g is a dict, <k>: list<t, v>
def graph2matrix_weight(g):
    n=len(g)
    m=np.array(np.zeros([n,n]))
    for k in g:
        for t,w in g[k]:
            m[k,t]=w
    return m
    
def graph2matrix_unweight(g):
    n=len(g)
    m=np.array(np.zeros([n,n]))
    for k in g:
        for t in g[k]:
            m[k,t]=1
    return m

def matrix2graph_weight(m):
    n=len(m)
    g={}
    for i in range(n):
        temp=[]
        for j in range(n):
            if m[i][j]!=0:
                temp.append(j)
        g[i]=temp
    return m


def main(ifolder, iprefix, ofolder, oprefix):
    g, n = common.load_graph_weight(ifolder, iprefix)
    m = graph2matrix_weight(g)
    if np.linalg.matrix_rank(m) == len(m):
        print('The original matrix is already invertible.')
    else:
        print('Generating invertible matrix')
        m = 0.5*(m + m.transpose())
        og = matrix2graph_weight(m)
        common.dump_graph_weight(ofolder, oprefix, n, og)

if __name__ == '__main__':
    argc = len(sys.argv)
    if argc <= 4:
        print('Make the matrix of a graph being invertible if it is not.')
        print('Usage: <input-path> <input-prefix> <output-path> <output-prefix>')
        exit()
    ifolder = sys.argv[1]
    iprefix = sys.argv[2]
    ofolder = sys.argv[3]
    oprefix = sys.argv[4]
    main(ifolder, iprefix, ofolder, oprefix)

