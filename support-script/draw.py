# -*- coding: utf-8 -*-

import re
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd


def set_small_figure(fontsize=12):
    plt.rcParams["figure.figsize"] = [4,3]
    plt.rcParams["font.size"] = fontsize


def set_large_figure(fontsize=16):
    plt.rcParams["figure.figsize"] = [6,4.5]
    plt.rcParams["font.size"] = fontsize

# %% basic compare

class Record():
    def __init__(self, name):
        self.name = name.upper()
        self.reversible = name.upper() in ['PR', 'KATZ', 'MC']
        self.t_r_spark = 0
        self.t_r_daiger = 0
        self.c_ratio = 0
        self.t_i_spark = 0
        self.t_i_daiger = 0
        
    def set_name(self, name):
        self.name = name
        self.reversible = name.upper() in ['PR', 'KATZ', 'MC']
        

def load_cmp_record(fn):
    data = pd.read_csv(fn,delimiter='\t')
    # name, t-r-spark, t-r-daiger, t-i-spark, t-i-daiger, std-i-spark, std-i-daiger
    res = []
    for d in data:
        if d[0][0] == '#' or d[1:].sum() == 0:
            continue
        r = Record(d[0])
        r.t_r_spark = d[1]
        r.t_r_daiger = d[2]
        r.t_i_spark = d[3]
        r.t_i_daiger = d[4]
        res.append(res)
    return res
    
def load_cmp(fn):
    name = []
    data = []
    with open(fn) as fin:
        # read the header
        fin.readline()
        # read the body
        for line in fin:
            if len(line) < 10 or line[0] == '#':
                continue
            l = line.split('\t')
            n = l[0].upper()
            d = [float(v) for v in l[1:]]
            name.append(n)
            data.append(d)
    return (name, np.array(data))

def draw_cmp(name, data, width=0.8, ncol=1, rotation=0, loc=None):
    ng = data.shape[0]
    nb = 2
    barWidth = width/nb
    off = -width/2 + barWidth/2
    x = np.arange(ng)
    y1 = data[:,0]
    y2 = data[:,1]
    e1 = data[:,2]
    e2 = data[:,3]
    plt.figure()
    plt.bar(x + off + barWidth*0, y1, barWidth, yerr=e1)
    plt.bar(x + off + barWidth*1, y2, barWidth, yerr=e2)
    plt.xticks(x, name, rotation=rotation)
    plt.xlabel('algorithm')
    plt.ylabel('running time (s)')
    plt.legend(['Spark', 'Daiger'], loc=loc)
    plt.tight_layout()

# %% delta ratio

def load_deltaratio(fn):
    return np.loadtxt(fn, delimiter='\t', skiprows=1)

def draw_deltaratio_line(data, loc=None):
    assert data.ndim == 2 and data.shape[1] == 3
    plt.figure()
    x=data[:,0]
    plt.plot(x, data[:,1:])
    plt.xlabel('ratio or changed edges')
    plt.ylabel('running time (s)')
    plt.legend(['Spark', 'Daiger'], loc=loc)
    plt.grid(True, linestyle='--')
    plt.tight_layout()

def draw_deltaratio_bar(data, width=0.8, ncol=1, rotation=0, loc=None):
    assert data.ndim == 2 and data.shape[1] == 3
    ng = data.shape[0]
    nb = 2
    barWidth = width/nb
    off = -width/2 + barWidth/2
    x = np.arange(ng)
    y1 = data[:,1]
    y2 = data[:,2]
    plt.figure()
    plt.bar(x + off + barWidth*0, y1, barWidth)
    plt.bar(x + off + barWidth*1, y2, barWidth)
    plt.xticks(x, data[:,0], rotation=rotation)
    plt.xlabel('ratio of changed edges')
    plt.ylabel('running time (s)')
    plt.legend(['Spark', 'Daiger'], loc=loc)
    plt.tight_layout()

# %% priority

def load_priority(fn):
    return np.loadtxt(fn, delimiter='\t', skiprows=1)

def get_log_ticks(low, high, base=1, exp=10):
    v = base
    c = 0
    while v >= low:
        v /= exp
        c -= 1
    v*=exp
    c+=1
    if base == 1:
        ptn = '$\mathdefault{'+str(exp)+'^{%d}}$'
    else:
        ptn = '$\mathdefault{'+str(base)+'x'+str(exp)+'^{%d}}$'
    values = []
    ticks = []
    i=c
    while base*exp**i <= high:
        values.append(base*exp**i)
        ticks.append(ptn % (i))
        i+=1
    return values, ticks
    

def draw_priority(data, rotation=0, ylow=None, yhigh=None):
    assert data.ndim == 2 and data.shape[1] == 2
    plt.figure()
    x=data[:,0]
    y=data[:,1]
    plt.plot(x,y)
    plt.xscale('log')
    xticks, xtickslabel = get_log_ticks(min(x), max(x))
    if 1 in xticks:
        idx = xticks.index(1)
        xtickslabel[idx] = '1 (RR)'
    plt.xticks(xticks, xtickslabel, rotation=rotation)
    plt.xlabel('queue size (portion of nodes)')
    plt.ylabel('running time (s)')
    plt.grid(True, linestyle='--')
    plt.tight_layout()
    

# %% scale

def load_scale(fn):
    return np.loadtxt(fn, delimiter='\t', skiprows=1)

def draw_scale_worker(data, speedup=False, loc=None):
    plt.figure()
    x=data[:,0]
    d=data[:,1]
    if speedup:
        y=d[0]/d
        ref=x/x[0]
        plt.plot(x, y, linestyle='-')
        plt.plot(x, ref, linestyle='--')
        plt.ylabel('speed up')
    else:
        y=d
        ref=d[0]*x[0]/x
        plt.plot(x, y, linestyle='-')
        plt.plot(x, ref, linestyle='--')
        plt.ylabel('running time (s)')
    plt.xlabel('# of workers')
    plt.legend(['Daiger', 'Optimal'], loc=loc)
    ylim=plt.ylim()
    plt.ylim((0, ylim[1]))
    plt.grid(True, linestyle='--')
    plt.tight_layout()


# %% main

