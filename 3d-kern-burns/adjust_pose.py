

import camerawrapper
import torch 
from torchvision.transforms.functional import to_pil_image
import numpy as np 
import cv2 

c = camerawrapper.CameraWrapper()

# c.setNearFar(400.0, 2000.0)
# c.getColorDepthData()
# npyImage, npyImageDepth = c.getColorImgNM(), c.getDepthImgNM()
# pil_depth_image = to_pil_image(torch.from_numpy(npyImageDepth.astype(np.float32)))
# pil_depth_image.save('depth.png')
# cv2.imwrite("color.png", npyImage)



import cv2
import numpy as np
import camerawrapper
from PIL import Image
import os 


while True:
    # gett rgb and depth data of current frame
    c.getColorDepthData()
    rgb_im = c.getColorImgNM()
    # depth_info = c.getDepthImgNM()
    # depth_hist_im = c.getHistImgNM()
    cv2.imshow('im', rgb_im)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break;

