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


