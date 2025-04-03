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

import matplotlib.pyplot as plt
import numpy as np

plt.rcParams.update({'font.size': 12})

#cmap = 'viridis'
cmap = 'jet'
#cmap = plt.cm.Blues

def domap(name, title, label, fig, ax, scale):
    print(name,'...')
    a = np.loadtxt(name, delimiter=",", dtype=int)
    a = a * scale
    #fig.figsize=(1024,1024)
    ax.imshow(a, cmap=cmap, interpolation='none')
    ax.set_title(title)
    ax.spines[:].set_visible(False)
    ax.figure.colorbar(plt.pcolor(a, cmap=cmap), ax=ax, label=label,fraction=0.046)#, pad=0.04)

fig,((ax1,ax2),(ax3,ax4))=plt.subplots(2,2)

domap("sent.count.nxn.heatmap.csv",'Sent','Count', fig, ax1, 1.0)
domap("sent.bytes.nxn.heatmap.csv",'Sent','MBytes', fig, ax2, 1.0e-6)
domap("recv.count.nxn.heatmap.csv",'Recv','Count', fig, ax3, 1.0)
domap("recv.bytes.nxn.heatmap.csv",'Recv','MBytes', fig, ax4, 1.0e-6)

fig.set_figwidth(8)
fig.set_figheight(6)
fig.tight_layout()
plt.savefig("heatmap.png", format="png", bbox_inches="tight")
#plt.show()

