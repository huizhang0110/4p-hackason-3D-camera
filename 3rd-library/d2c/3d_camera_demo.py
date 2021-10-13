import cv2
import numpy as np
import camerawrapper
from PIL import Image
import os 

color_fpath = "color.png"
depth_fpath = "depth"
output_fpath = "autozoom.mp4"

c = camerawrapper.CameraWrapper()
c.getColorDepthData()
color, depth = c.getColorImgNM(), c.getDepthImgNM()
cv2.imwrite(color_fpath, color)
np.save(depth_fpath, depth)

# os.system(f"python autozoom.py --in {color_fpath} --inDepth {depth_fpath} --out {output_fpath}")
