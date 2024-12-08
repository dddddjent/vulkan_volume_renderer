import sys
import numpy as np

input_filename = sys.argv[1]
output_path = sys.argv[2]


original_field = np.load(input_filename).astype(np.float32)


def fire_process(x):
    if x > 2.6:
        return x / 30.0
    else:
        return 0.0


fire = original_field.copy()
fire = np.vectorize(fire_process)(fire).astype(np.float32)


def smoke_process(x):
    if x < 2.6 and x >= 1.0:
        return x / 30.0
    else:
        return 0.0


smoke = original_field.copy().astype(np.float32)
smoke = np.vectorize(smoke_process)(smoke).astype(np.float32)

np.save(output_path + "/fire.npy", fire)
np.save(output_path + "/smoke.npy", smoke)
