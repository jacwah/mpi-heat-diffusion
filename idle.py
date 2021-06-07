import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits import mplot3d
from matplotlib import collections
from matplotlib import colors
import struct
import math

metafile = open("heat-meta.bin", "rb")
X, Y, T, nprocs = struct.unpack("4i", metafile.read(16))
n = int(math.sqrt((X*Y) / nprocs))

print(f"Domain: ({X}, {Y}), {T} timesteps, {nprocs} processes")

lines = []
lines2 = []
color2 = []

xn = X//n
yn = Y//n

lens = []

for i in range(xn):
    for j in range(yn):
        with open(f"idle-{i}-{j}.bin", "rb") as f:
            ints = struct.unpack(f"{2*T}q", f.read(T*2*8))
            for k in range(T):
                size = ints[2*k+0]-ints[2*k+1]
                lens.append(size)
                if size >= 1.34857622e+08:
                    lines.append(((i, j, ints[2*k+1]), (i, j, ints[2*k+0])))
                # lines2.append(((j+i/xn, ints[2*k+1]), (j+i/xn, ints[2*k+0])))
                # color2.append(colors.hsv_to_rgb((i/xn, 1.0, 1.0)))

lens = np.array(lens)
percs = np.array([90, 99, 99.9])
print(np.percentile(lens, percs))

fig = plt.figure()

# ax = fig.add_subplot()
# linecoll = collections.LineCollection(lines2, color=color2)
# ax.set_xlim(0, yn-1)
# ax.set_ylim(0, np.max(lines))
# ax.set_xlabel("process y")
# ax.set_ylabel("cycles")
# ax.set_title("Idle periods")
# ax.set_xticks(list(range(16)))

ax = fig.add_subplot(projection='3d', title="99 percentile idle periods")
ax.set_xlim(0, xn-1)
ax.set_ylim(0, yn-1)
ax.set_zlim(0, np.max(lines))
ax.set_xlabel("process x")
ax.set_ylabel("process y")
ax.set_zlabel("cycles")
linecoll = mplot3d.art3d.Line3DCollection(lines, lw=1)

ax.add_collection(linecoll)
plt.show()
