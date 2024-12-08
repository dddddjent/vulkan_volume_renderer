import sys
import numpy as np

input_filename = sys.argv[1]
output_path = sys.argv[2]


original_field = np.load(input_filename)


def fire_process(x):
    if x < 950:
        return 0
    else:
        return x


fire = original_field.copy()
fire = np.vectorize(fire_process)(fire).astype(np.float32)


def smoke_process(x):
    if x >= 950 or x < 674:
        return 0
    else:
        return x


smoke = original_field.copy()
smoke = np.vectorize(smoke_process)(smoke).astype(np.float32)
smoke_min, smoke_max = np.min(smoke), np.max(smoke)
smoke = (smoke - smoke_min) / (smoke_max - smoke_min)
smoke *= 100

np.save(output_path + "/fire.npy", fire)
np.save(output_path + "/smoke.npy", smoke)
