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

# %% data 

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


# %% draw

def draw_cmp(name, data, width=0.8, ncol=1, rotation=0, loc=None):
    ng = data.shape[0]
    nb = 2
    barWidth = width/nb
    off = -width/2 + barWidth/2
    x = np.arange(ng)
    y1 = data[:,2]
    y2 = data[:,3]
    e1 = data[:,4]
    e2 = data[:,5]
    plt.figure()
    plt.bar(x + off + barWidth*0, y1, barWidth, yerr=e1)
    plt.bar(x + off + barWidth*1, y2, barWidth, yerr=e2)
    plt.xticks(x, name, rotation=rotation)
    plt.xlabel('algorithm')
    plt.ylabel('running time (s)')
    plt.legend(['Spark', 'Daiger'], loc=loc)
    plt.tight_layout()


# %% main

