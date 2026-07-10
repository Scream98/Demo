#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import subprocess
import shutil
from PIL import Image
def osSystem(cmd):
    os.system(("echo " + cmd))
    return subprocess.call(cmd)
    
def checkIconSize(path, sizeW, sizeH, excludenames):
    allFile = os.listdir(path)
    sourcePath = ""
    fail = False
    for file in allFile: 
        fullPath = os.path.join(path, file)
        if os.path.isdir(fullPath):
            checkIconSize(fullPath, sizeW, sizeH, excludenames)
            continue
        if file in excludenames:
            continue
        if '.png' not in file and '.jpg' not in file:
            continue 
        print("checking "+fullPath)
        im = Image.open(fullPath)
        if (im.size[0] > sizeW or im.size[1] > sizeH):
            print("icon check Fail!!!! " + fullPath + " size is invalid!!!standard small than:" + str(sizeW) + "X" + str(sizeH))
            fail = True
        im.close()
    if fail:
        exit(-1)
            
if __name__=='__main__':
    checkPath = sys.argv[1]
    sizeW = sys.argv[2]
    sizeH = sys.argv[3]
    excludenames = []
    if len(sys.argv) > 4:
        excludenames = sys.argv[4].split(",")
    checkIconSize(checkPath, int(sizeW), int(sizeH), excludenames)