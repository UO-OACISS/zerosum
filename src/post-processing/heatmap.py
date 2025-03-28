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
import matplotlib.colors as colors
import matplotlib.cbook as cbook
from matplotlib import cm
import numpy as np

#cmap = 'viridis'
cmap = 'PuBu_r'
#cmap = plt.cm.Blues

def domap(name, title, label):
    print(name,'...')
    a = np.loadtxt(name, delimiter=",", dtype=int)
    #fig.figsize=(512,512)
    fig, ax = plt.subplots(1,1)
    ax.imshow(a, cmap=cmap, interpolation='none')
    ax.set_title(title)
    minval = np.min(a)
    maxval = np.max(a)
    ax.figure.colorbar(plt.pcolor(a, cmap=cmap), ax=ax, label=label)
    fig.tight_layout()
    #plt.savefig(name+".pdf", format="pdf", bbox_inches="tight")
    plt.show()

domap("sent.count.nxn.heatmap.csv",'Sent','Count')
domap("sent.bytes.nxn.heatmap.csv",'Sent','Bytes')
domap("recv.count.nxn.heatmap.csv",'Recv','Count')
domap("recv.bytes.nxn.heatmap.csv",'Recv','Bytes')


