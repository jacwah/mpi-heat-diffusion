import numpy as np
import matplotlib.pyplot as plt

# parameters: 5040 5040 100

t = np.array([
    1.163884e+02, 3.638901e+01, 1.698895e+01, 1.225009e+01, 1.187987e+01,
    8.581875e+00, 7.486608e+00, 5.572435e+00, 5.091564e+00, 4.750127e+00,
    4.160318e+00, 3.534188e+00, 3.141682e+00, 3.435462e+00])

n = np.array([1, 4, 9, 16, 25, 36, 49, 64, 81, 100, 144, 196, 225, 256])

plt.xlabel("Number of processes")
plt.ylabel("Execution time (s)")
plt.loglog(n, t, marker="o")
plt.title("Strong scaling")

plt.show()
