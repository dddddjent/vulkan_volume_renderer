import sys
import numpy as np

output_file = sys.argv[1]

arr = np.zeros((128, 128, 128)).astype(np.float32)

arr += 2

np.save(output_file, arr)
