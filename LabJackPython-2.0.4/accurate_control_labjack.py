# -*- coding: utf-8 -*-
"""
Created on Fri Sep 16 00:04:06 2022

@author: 28953
"""

import time
import u3

d = u3.U3()
print(d.configU3())

def sleep(duration, get_now=time.perf_counter):
    now = get_now()
    end = now + duration
    while now < end:
        now = get_now()
        
        
count = 0
T1 = time.perf_counter()
d.setFIOState(4, state = 1)
sleep(0.03)
while (count < 1000):

    d.setFIOState(5, state = 1)
    sleep(0.1977)
    d.setFIOState(5, state = 0)
    sleep(0.4)
    count = count + 1

T2 =time.perf_counter()
print('running time:%s ms' % ((T2 - T1)*1000))


d.setFIOState(4, state = 0)