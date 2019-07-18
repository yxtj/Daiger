# -*- coding: utf-8 -*-
"""
Created on Wed Jul 17 11:07:12 2019

@author: yanxi
"""

import re
import numpy as np
import matplotlib.pyplot as plt


def loadLog(fn):
    pat=re.compile(r'.+\] Time: ([\d.]+) current progress: \(([\d.e+-]+),0\) improvement: \(([\d.e+-]+),0\)')
    res=[];
    with open(fn) as f:
        for line in f.readlines():
            m=pat.match(line)
            if m:
                res.append([float(m.group(1)), float(m.group(2)), float(m.group(3))])
    return np.array(res)

def drawLog(fnList):
    plt.figure()
    for fn in fnList:
        d=loadLog(fn)
        plt.plot(d[:,0],d[:,1])
    plt.xlabel('time')
    plt.ylabel('progress')
    plt.legend(fnList)





r10=loadLog('t5-3-rr-1')
r5=loadLog('t5-3-rr-0.5')
r2=loadLog('t5-3-rr-0.2')
r1=loadLog('t5-3-rr-0.1')

p5=loadLog('t5-3-p-0.5')
p2=loadLog('t5-3-p-0.2')
p1=loadLog('t5-3-p-0.1')

q1=loadLog('t5-3-p1-0.1')
q2=loadLog('t5-3-p1-0.2')
q5=loadLog('t5-3-p1-0.5')
