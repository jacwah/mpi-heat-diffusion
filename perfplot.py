import numpy as np
import matplotlib.pyplot as plt

t = np.array([float(x) for x in open("heat-time", "r").readlines()])

n = np.array([1, 4, 9, 16, 25, 36, 49, 64, 81, 100, 121, 144, 196, 225, 256])

fig, (ax1, ax2) = plt.subplots(1, 2)

ax1.set_title("Log-log execution time")
ax1.set_xlabel("Number of processes")
ax1.set_ylabel("Execution time (s)")
ax1.loglog(n, t, marker="o")

ax2.set_title("Speedup")
ax2.set_xlabel("Number of processes")
ax2.set_ylabel("Speedup")
ax2.plot(n, t[0]/t, marker="o")
ax2.plot([0, 100], [0, 100], ls="--")

plt.show()
