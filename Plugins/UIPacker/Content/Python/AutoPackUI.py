#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import subprocess
import shutil

MobileSize = 2048
PCSize = 4096
tool = '"{rootDir}/../TexturePacker/bin/TexturePacker.exe" --format unreal-paper2d --force-publish --trim-sprite-names --trim-mode None --png-opt-level 0 --extrude 1 --disable-rotation --size-constraints NPOT --pack-mode Fast --max-width {maxWidth} --max-height {maxHeight} --data "{target}/atlas.paper2dsprites" --sheet "{target}/atlasTex.png" {source}'
CopyPath = "__UIPackerTmp"

def osSystem(cmd):
    os.system(("echo " + cmd))
    # TexturePacker 首次运行需同意许可证，自动处理
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True, input="agree\n")
    print("Command output:")
    print(result.stdout)
    if result.stderr:
        print("Command errors:")
        print(result.stderr)
    return result.returncode

def UIPacker(inPath, outPath, isMobile):
    if not os.path.exists(inPath):
        return 0

    print("[UIPacker] inPath: {0}, outPath: {1}".format(inPath, outPath))

    # 检查是否有子目录，有则拒绝打图集（不允许带子目录的目录被打图集）
    for item in os.listdir(inPath):
        if os.path.isdir(os.path.join(inPath, item)):
            print("[UIPacker] ERROR: directory has sub-folders, not allowed: " + inPath)
            print("[UIPacker] Please select a leaf folder (without sub-folders) instead.")
            return -1

    # 创建对应的CopyPath, 把非子目录的资源文件硬链接到CopyPath，打图集时只传递CopyPath替代传递若干个图集路径
    copyFolderPath = os.path.join(inPath, CopyPath)

    if os.path.exists(copyFolderPath):
        shutil.rmtree(copyFolderPath)
    os.makedirs(copyFolderPath, exist_ok=True)
    print("[UIPacker] create copy folder path: " + copyFolderPath)

    allFile = os.listdir(inPath)
    anySourceFileLinked= False
    for lists in allFile:
        lists = os.path.join(inPath, lists)
        if '.png' not in lists and '.jpg' not in lists:
            continue

        # 把需要的文件通过创建硬链接集中到一个folder，打图集直接传递folder路径替代传递若干个图集路径
        anySourceFileLinked = True
        copyFilePath = os.path.join(copyFolderPath, os.path.basename(lists))
        os.link(os.path.abspath(lists), os.path.abspath(copyFilePath))
        print("[UIPacker] link source file: " + lists + " to: " + copyFilePath)

    if not anySourceFileLinked:
        print("[UIPacker] No PNG/JPG files found in: " + inPath)
        return -1
    
    maxSize = MobileSize
    if not isMobile:
        maxSize = PCSize

    executeCmd = tool.replace("{rootDir}", dir).replace("{target}", outPath).replace("{source}", copyFolderPath).replace("{maxWidth}", str(maxSize)).replace("{maxHeight}", str(maxSize))

    print(executeCmd)
    errorlevel = osSystem(executeCmd)
    if errorlevel != 0:
        print("[UIPacker] TexturePacker failed with error code: " + str(errorlevel))
        return errorlevel

    # 验证输出文件
    atlasPng = os.path.join(outPath, "atlasTex.png")
    atlasData = os.path.join(outPath, "atlas.paper2dsprites")
    if not os.path.exists(atlasPng):
        print("[UIPacker] ERROR: output file not created: " + atlasPng)
        return -1
    if not os.path.exists(atlasData):
        print("[UIPacker] ERROR: output file not created: " + atlasData)
        return -1
    print("[UIPacker] OK: atlasTex.png size=" + str(os.path.getsize(atlasPng)) +
          ", atlas.paper2dsprites size=" + str(os.path.getsize(atlasData)))
    return 0

if __name__=='__main__':
    list = sys.argv
    # 模拟数据
    # list = ["","","","", ""]
    # list[0] = "Plugins/UIPacker/Content/Python/AutoPackUI.py"
    # list[1] = "D:/Project/Content/UI/Textures/MyIcons"
    # list[2] = "D:/Project/Content/UI/SpriteSheet/MyIcons"
    # list[3] = "false"
    # list[4] = "true"

    dir = os.path.abspath(os.path.dirname(list[0]))
    inputPath = list[1]
    savedPath = list[2]
    isDynamic = list[3] == "true"
    isMobile = list[4] == "true"

    print("[UIPacker] dir: {0}, inputPath: {1}, savedPath: {2}, isDynamic: {3}, isMobile: {4}".format(dir, inputPath, savedPath, isDynamic, isMobile))
    
    errorlevel = UIPacker(inputPath, savedPath, isMobile)

    copyFolderPath = os.path.join(inputPath, CopyPath)
    if os.path.exists(copyFolderPath):
        shutil.rmtree(copyFolderPath)
        print("[UIPacker] remove copy folder path: " + copyFolderPath)

    if errorlevel != 0:
        print("UIPacker failedpath:" + inputPath + " target:" + savedPath)
        sys.exit(-1)
