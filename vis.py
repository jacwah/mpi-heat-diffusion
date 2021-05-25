import numpy as np
import matplotlib.pyplot as plt
import struct
import math

metafile = open("heat-meta.bin", "rb")
X, Y, T, nprocs = struct.unpack("4i", metafile.read(16))
n = int(math.sqrt((X*Y) / nprocs))

print(f"Domain: ({X}, {Y}), {T} timesteps, {nprocs} processes")

theta = np.zeros((T, X, Y))

for i in range(X//n):
    for j in range(Y//n):
        with open(f"heat-{i}-{j}.bin", "rb") as f:
            doubles = struct.unpack(f"{T*n*n}d", f.read(T*n*n*8))
            tile = np.reshape(doubles, (T, n, n), "C")
            theta[:, i*n:(i+1)*n,j*n:(j+1)*n] = tile

plt.imshow(theta[-1])
plt.colorbar()

plt.show()
