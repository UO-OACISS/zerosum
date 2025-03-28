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

#findme = "user:"
#filename = "user.pdf"
#findme = "system:"
#filename = "system.pdf"
#findme = "idle:"
#filename = "idle.pdf"
#findme = "Device Busy %:"
#filename = "busy.pdf"
#findme = "Energy Average (J):"
#filename = "joules.pdf"
#findme = "Voltage (mV):"
#filename = "volts.pdf"
#findme = "Used VRAM Bytes:"
#filename = "vram.pdf"
findme = "Temperature (C):"
filename = "temp.pdf"
avg = 'average:'

def parseFile(f):
    # reading the file
    data = f.read()

    data_into_list = data.split("\n")
    # match *** Final thread summary: ***
    for line in data_into_list:
        line = line.strip()
        if findme in line:
            line = line[len(findme):].strip()
            line = line[:line.find(avg)]
            row = ast.literal_eval(line)
            #row = csv.reader(io.StringIO(line), delimiter=',')
            return list(row)

def iterateFiles():
    rows = []
    files = sorted(glob.glob('zs.*.log'))
    numfiles = len(files)
    for f in files:
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
    maxval = max_value(rows)
    minval = min_value(rows, maxval)
    max_x, max_y = closestDivisors(size)
    fig,axs = plt.subplots(max_x, max_y)
    fig.set_figheight(10)
    fig.set_figwidth(16)
    steps = range(len(rows[0]))
    index = 0
    for x in range(max_x):
        for y in range(max_y):
            axs[x,y].plot(steps, rows[index], label=str(x), color='black', linewidth=0.25)
            #axs[x,y].axhline(c='grey', alpha=0.5)
            #axs[x,y].xaxis.set_major_locator(ticker.NullLocator())
            #axs[x,y].yaxis.set_major_locator(ticker.NullLocator())
            axs[x,y].set_ylim([minval,maxval])
            axs[x,y].set_facecolor('#dddddd')
            axs[x,y].grid(color='#eeeeee')
            axs[x,y].set_xticklabels([])
            axs[x,y].set_yticklabels([])
            for pos in ['right', 'left', 'top', 'bottom']:
                axs[x,y].spines[pos].set_visible(False)
            index += 1
    #plt.tight_layout()
    title = findme[:-1] + ", range: [" + format(minval,",") + ", " + format(maxval,",") + "]"
    fig.suptitle(title, fontsize = 32)
    plt.savefig(filename, format="pdf", bbox_inches="tight")
    #plt.show()

def main():
    global filename, findme
    findme_list = ["Device Busy %:", "Energy Average (J):", "Voltage (mV):", "Used VRAM Bytes:", "Temperature (C):", "Clock Frequency, GLX (MHz):", "Memory Busy %:"]
    filename_list = ["busy.pdf", "joules.pdf", "volts.pdf", "vram.pdf", "temp.pdf", "clock.pdf", "membusy.pdf"]
    for l, f in zip(findme_list, filename_list):
        findme = l
        filename = f
        print("doing", findme, '...')
        df = iterateFiles()
        graphit(df)
    print('done.')

if __name__ == '__main__':
    main()

