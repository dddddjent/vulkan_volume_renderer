import numpy as np
from PIL import Image
import sys

arr: np.ndarray = np.load(sys.argv[1])
arr = arr[..., [2, 1, 0]]

Image.fromarray(arr).save(sys.argv[2])
