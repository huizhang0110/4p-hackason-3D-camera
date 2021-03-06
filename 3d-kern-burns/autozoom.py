#!/usr/bin/env python

import torch
import torchvision

import base64
import cupy
import cv2
import flask
import getopt
import gevent
import gevent.pywsgi
import glob
import h5py
import io
import math
import moviepy
import moviepy.editor
import numpy
import os
import random
import re
import scipy
import scipy.io
import shutil
import sys
import tempfile
import time
import urllib
import zipfile

##########################################################

assert(int(str('').join(torch.__version__.split('.')[0:2])) >= 12) # requires at least pytorch version 1.2.0

torch.set_grad_enabled(False) # make sure to not compute gradients for computational performance

torch.backends.cudnn.enabled = True # make sure to use cudnn for computational performance

##########################################################

objCommon = {}

exec(open('./common.py', 'r').read())

exec(open('./models/disparity-estimation.py', 'r').read())
exec(open('./models/disparity-adjustment.py', 'r').read())
exec(open('./models/disparity-refinement.py', 'r').read())
exec(open('./models/pointcloud-inpainting.py', 'r').read())

##########################################################

arguments_strIn = './images/corridor_rgb.png'
arguments_strOut = './autozoom.mp4'
arguments_strInDepth = './images/corridor_depth.png'

for strOption, strArgument in getopt.getopt(sys.argv[1:], '', [ strParameter[2:] + '=' for strParameter in sys.argv[1::2] ])[0]:
    if strOption == '--in' and strArgument != '': arguments_strIn = strArgument # path to the input image
    if strOption == '--out' and strArgument != '': arguments_strOut = strArgument # path to where the output should be stored
    if strOption == '--inDepth' and strArgument != '': arguments_strInDepth = strArgument # path to where the output should be stored
# end

##########################################################

if __name__ == '__main__':
    import numpy as np 
    import cv2 
    import camerawrapper

    npyImage = cv2.imread(filename=arguments_strIn, flags=cv2.IMREAD_COLOR)
    # npyImageDepth = cv2.imread(filename=arguments_strInDepth, flags=cv2.IMREAD_GRAYSCALE)
    npyImageDepth = np.load(arguments_strInDepth).astype(np.float32)
    npyImageDepth[npyImageDepth <= 1] = npyImageDepth.max()

    # c = camerawrapper.CameraWrapper()
    # c.getColorDepthData()
    # npyImage, npyImageDepth = c.getColorImgNM(), c.getDepthImgNM()
    # cv2.imwrite('first.png', npyImage)

    intWidth = npyImage.shape[1]
    intHeight = npyImage.shape[0]

    fltRatio = float(intWidth) / float(intHeight)

    intWidth = min(int(1024 * fltRatio), 1024)
    intHeight = min(int(1024 / fltRatio), 1024)

    npyImage = cv2.resize(src=npyImage, dsize=(intWidth, intHeight), fx=0.0, fy=0.0, interpolation=cv2.INTER_AREA)
    npyImageDepth = cv2.resize(src=npyImageDepth, dsize=(intWidth, intHeight), fx=0.0, fy=0.0, interpolation=cv2.INTER_AREA)

    process_load(npyImage, npyImageDepth, {})

    objFrom = {
        'fltCenterU': intWidth / 2.0,
        'fltCenterV': intHeight / 2.0,
        'intCropWidth': int(math.floor(0.97 * intWidth)),
        'intCropHeight': int(math.floor(0.97 * intHeight))
    }

    objTo = process_autozoom({
        'fltShift': 100.0,
        'fltZoom': 1.25,
        'objFrom': objFrom
    })

    npyResult = process_kenburns({
        'fltSteps': numpy.linspace(0.0, 1.0, 75).tolist(),
        'objFrom': objFrom,
        'objTo': objTo,
        'boolInpaint': True
    })

    npyResult = [npyImage] + npyResult
    frames = [ npyFrame[:, :, ::-1] for npyFrame in npyResult + list(reversed(npyResult))[1:] ]
    H, W = frames[0].shape[:2]
    # moviepy.editor.ImageSequenceClip(
    #     sequence=[ 
    #         npyFrame[:, :, ::-1] for npyFrame in npyResult + list(reversed(npyResult))[1:] 
    #     ], 
    #     fps=25
    # ).write_videofile(arguments_strOut)

    video_writer = cv2.VideoWriter(
        arguments_strOut,
        fourcc=cv2.VideoWriter_fourcc(*'MJPG'), 
        fps=25,
        frameSize=(W, H),
    )
    for frame in frames:
        video_writer.write(frame[:, :, ::-1])
    video_writer.release()
    print("finish")

# # end
