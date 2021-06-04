import numpy as np
import matplotlib.pyplot as plt

t = np.array([float(x) for x in open("heat-time", "r").readlines()])

n = np.array([1, 4, 9, 16, 25, 36, 49, 64, 81, 100, 121, 144, 196, 225, 256])

plt.xlabel("Number of processes")
plt.ylabel("Execution time (s)")
plt.loglog(n, t, marker="o")
plt.title("Strong scaling")

plt.savefig("perfplot.png")
plt.show()
