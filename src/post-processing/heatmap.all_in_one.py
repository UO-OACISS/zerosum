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

