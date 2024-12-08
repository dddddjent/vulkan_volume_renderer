import sys
import pyvista as pv
import numpy as np

input_filename = sys.argv[1]
output_filename = sys.argv[2]

fields = pv.read(input_filename)

temperature = fields["temperature"]

# swap x and z
xs, ys, zs = 256, 128, 128
t1 = temperature.copy()
for i in range(xs):
    for j in range(ys):
        for k in range(zs):
            t1[i * (zs * ys) + j * zs + k] = temperature[k * (xs * ys) + j * xs + i]

# swap y and x
temperature = t1.copy()
xs, ys, zs = 128, 128, 256
for i in range(xs):
    for j in range(ys):
        for k in range(zs):
            t1[k * (xs * ys) + i * ys + j] = temperature[k * (xs * ys) + j * xs + i]

t1 = np.reshape(t1, (128, 128, 256))

t1 += 273 + 400

np.save(output_filename, t1)
