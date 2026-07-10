#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import subprocess
import shutil
import json

from PIL import Image
importSettingTemplate = ""
outputSetting = ""
workDir = ""

#差量导出配置文件名称
diffConfigName = "diff.json"

def osSystem(cmd):
    os.system(("echo " + cmd))
    return subprocess.call(cmd)

def CopyTexture(sourcePath, targetPath, limitSize, cleanFolder, exceptList={}):
    if os.path.exists(targetPath) and cleanFolder:
        print("[CopyTexture]delete " + targetPath)
        shutil.rmtree(targetPath)
    if os.path.exists(sourcePath):
        for root, dirs, files in os.walk(sourcePath):
            for name in files:
                if bool(exceptList) and (not os.path.splitext(name)[0] in exceptList.keys()):
                    continue
                if(".png" in name or ".jpg" in name):
                    im = Image.open(os.path.join(root, name))
                    if (im.size[0] > limitSize or im.size[1] > limitSize):
                        print("[CopyTexture]Copy Fail!!!!" + os.path.join(root, name) + " is too large!!!standard:" + str(limitSize))
                        im.close()
                        exit(-1)
                    im.close()
        shutil.copytree(sourcePath, targetPath)
    print("CopyTexture success:" + sourcePath + " to " + targetPath)

def genImportSettingInfo(path, exceptList={}):
    fr = open(importSettingTemplate, "r")
    content = fr.read()
    fr.close()

    fileList = ""
    for root, dirs, files in os.walk(path):
        for name in files:
            if ".uasset" in name:
                continue
            if bool(exceptList) and (not os.path.splitext(name)[0] in exceptList.keys()):
                continue
            filename = os.path.join(root, name)
            destname  = root[root.index("/Content/"):].replace("/Content/", "/Game/")
            fileList += content.replace("{__fileContent__}", filename.replace("\\","\\\\").replace("/","\\\\")).replace("{__fileDest__}", destname.replace("\\","/")) + ',\n'
    fileList = fileList[:-2]
    return fileList

def createImportSetting(path, outputSetting, expectContent=None):
    outer = '{\n'\
            '    "ImportGroups":[\n'\
            '         {__mainContent__}\n'\
            '    ]\n'\
            '}'

    fileList = expectContent if expectContent != None else genImportSettingInfo(path)
    fw = open(outputSetting, "w")
    fw.write(outer.replace("{__mainContent__}", fileList))
    fw.close()


if __name__=='__main__':
    # 模拟数据
    # sys.argv = ["", "", "", "", "", ""]
    # sys.argv[0] = "D:/Company/KDD/KDDApp/Plugins/UIPacker/Content/Python/ImportAsset.py"
    # sys.argv[1] = ""
    # sys.argv[2] = "D:/Company/KDD/KDDApp/Content/KDDApp/Base/UI/DynamicLoadRes/Textures/Icons/HeadIcons_Tex"
    # sys.argv[3] = "Texture"
    # sys.argv[4] = "true"
    # sys.argv[5] = 256

    workDir = os.path.abspath(os.path.dirname(sys.argv[0]))
    selectFolder = sys.argv[1]
    importType = sys.argv[3]
    cleanFolder = sys.argv[4] == "true"
    limitSize = int(sys.argv[5])

    diffConfig = {}
    isExportAll = True
    diffConfigPath = os.path.join(workDir, diffConfigName)
    if os.path.exists(diffConfigPath):
        with open(diffConfigPath, 'r') as read_f:
            diffConfig = json.load(read_f)
            isExportAll = diffConfig['isExportAll']

    importSettingTemplate = importType + "ImportSettingTemplate.txt"
    importSettingTemplate = os.path.join(workDir,importSettingTemplate)
    outputSetting = importType + "ImportSetting.json"
    outputSetting = os.path.join(workDir,outputSetting)

    if isExportAll:
        print("[CopyTexture] ExportAll!")
        CopyTexture(sys.argv[1], sys.argv[2], limitSize, cleanFolder)
        createImportSetting(sys.argv[2], outputSetting)
    else:
        print("[CopyTexture] Export with last export revision diff!")
        fileList = ""
        considerFolderMap = {
            "diffFolder": diffConfig['diffFolder'] or [],
            "addFolder": diffConfig['addFolder'] or [],
            "delFolder": diffConfig['delFolder'] or []
        }
        for keyname, considerFolder in considerFolderMap.items():
            for unit in considerFolder:
                sourcePath = unit['source'] or ""
                targetPath = unit['target'] or ""
                if sourcePath == "" or targetPath == "":
                    continue
                _cleanFolder = True if keyname == "delFolder" else cleanFolder
                _exceptList = {} if keyname == "addFolder" else unit['changelist']
                CopyTexture(sourcePath, targetPath, limitSize, _cleanFolder, _exceptList)
                curFileList = genImportSettingInfo(targetPath, _exceptList)
                if len(fileList) > 0 and len(curFileList) > 0:
                    fileList += ",\n" + curFileList
                elif len(fileList) <= 0 and len(curFileList) > 0:
                    fileList += curFileList
        createImportSetting(sys.argv[2], outputSetting, fileList)
        # 存粹的删除动作吧，会把整个Root目录给干掉了，这儿恢复一下目录，便于bat和svn对文件夹操作
        for unit in considerFolderMap["delFolder"]:
            targetPath = unit['target'] or ""
            if targetPath != "":
                os.makedirs(targetPath)
    print("CopyTexture Finish!")
