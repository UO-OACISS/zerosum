#!/usr/bin/env python3

"""
# MIT License
#
# Copyright (c) 2023-2025 University of Oregon, Kevin Huck
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
"""

import os.path
import sys
import glob
import io
import csv
import ast
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import math

findme1 = "user:"
findme2 = "system:"
findme3 = "idle:"
findme4 = "iowait:"
filename = "hwt.pdf"
hwt_index = 0
#findme = "Device Busy %:"
#filename = "busy.pdf"
#findme = "Energy Average (J):"
#filename = "joules.pdf"
#findme = "Voltage (mV):"
#filename = "volts.pdf"
#findme = "Used VRAM Bytes:"
#filename = "vram.pdf"
#findme = "Temperature (C):"
#filename = "temp.pdf"
avg = 'average:'

def parseFile(f):
    # reading the file
    data = f.read()

    data_into_list = data.split("\n")
    # match *** Final thread summary: ***
    found = 0
    index = -1
    user = []
    system = []
    idle = []
    incpu = False
    for line in data_into_list:
        line = line.strip()
        if "Hardware Summary:" in line:
            incpu = True
        elif incpu and "CPU" in line:
            print(line)
            index += 1
        elif index == hwt_index and findme1 in line:
            line = line[len(findme1):].strip()
            line = line[:line.find(avg)]
            tmp = ast.literal_eval(line)
            user = list(tmp)
            found += 1
        elif index == hwt_index and findme2 in line:
            line = line[len(findme2):].strip()
            line = line[:line.find(avg)]
            system = list(ast.literal_eval(line))
            found += 1
        elif index == hwt_index and findme3 in line:
            line = line[len(findme3):].strip()
            line = line[:line.find(avg)]
            idle = list(ast.literal_eval(line))
            found += 1
        elif index == hwt_index and findme4 in line:
            line = line[len(findme4):].strip()
            line = line[:line.find(avg)]
            iowait = list(ast.literal_eval(line))
            found += 1
        if found == 4:
            metrics = []
            metrics.append(user)
            metrics.append(system)
            metrics.append(idle)
            metrics.append(iowait)
            return metrics

def iterateFiles():
    rows = []
    files = sorted(glob.glob('zs.*.log'))
    numfiles = len(files)
    for f in files:
        print(f)
        with open(f, "r") as f_in:
            rows.append(parseFile(f_in))
    return rows

def max_value(rows):
    maxval = 0
    for row in rows:
        tmpmax = max(row)
        if tmpmax > maxval:
            maxval = tmpmax
    return maxval

def min_value(rows, maxval):
    minval = maxval
    for row in rows:
        tmpmin = min(row)
        if tmpmin < minval:
            minval = tmpmin
    return minval

def closestDivisors(n):
    a = round(math.sqrt(n))
    while n%a > 0: a -= 1
    return a,n//a

def graphit(rows):
    size = len(rows)
    maxval = 100
    minval = 0
    max_x, max_y = closestDivisors(size)
    fig,axs = plt.subplots(max_x, max_y)
    fig.set_figheight(10)
    fig.set_figwidth(16)
    steps = range(len(rows[0][0]))
    index = 0
    for x in range(max_x):
        for y in range(max_y):
            axs[x,y].stackplot(steps, rows[index][0], rows[index][1], rows[index][2], rows[index][3], labels=['user','system','idle','iowait'])
            #axs[x,y].stackplot(steps, rows[index][0], rows[index][1],  labels=['user','system'])
            #axs[x,y].axhline(c='grey', alpha=0.5)
            axs[x,y].xaxis.set_major_locator(ticker.NullLocator())
            axs[x,y].yaxis.set_major_locator(ticker.NullLocator())
            axs[x,y].set_ylim([minval,maxval])
            #axs[x,y].set_facecolor('#cccccc')
            #axs[x,y].grid(color='#eeeeee')
            #axs[x,y].set_xticklabels([])
            #axs[x,y].set_yticklabels([])
            for pos in ['right', 'left', 'top', 'bottom']:
                axs[x,y].spines[pos].set_visible(False)
            index += 1
    #plt.tight_layout()
    title = "HWT utilization"
    fig.suptitle(title, fontsize = 32)
    plt.savefig(filename, format="pdf", bbox_inches="tight")
    #plt.show()

def main():
    df = iterateFiles()
    graphit(df)
    print('done.')

if __name__ == '__main__':
    main()

