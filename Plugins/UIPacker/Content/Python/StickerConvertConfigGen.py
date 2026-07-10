#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import json

genPath = ""
sourcePath = ""
distPath = ""

def CollectConvertInfo():
    infos = []
    rootPath = os.path.abspath(genPath)
    for root, dirs, files in os.walk(rootPath):
        for file in files:
            fileSplit = os.path.splitext(file)
            framesPerSecond = -1
            if fileSplit[1] and fileSplit[1] == '.txt':
                try:
                    framesPerSecond = int(fileSplit[0])
                except:
                    continue
                tmpSourceFolderPath = root.replace(genPath, sourcePath).replace(
                    "\\", "/").replace(genPath, sourcePath)
                # print(tmpSourceFolderPath)
                tmpDistPath = distPath.replace("\\", "/")
                infos.append({'sourceFolderPath': tmpSourceFolderPath, 'distPath': tmpDistPath,
                                'framesPerSecond': framesPerSecond})
                break
    configPath = os.path.abspath(__file__) + '/../stickerGenConfig.json'
    with open(configPath, 'w', encoding='utf-8') as f:
        json.dump({'ConvertGroups': infos}, f, indent=4, ensure_ascii=False)


if __name__ == "__main__":
    if len(sys.argv) > 3:
        genPath = sys.argv[1].replace('\\', '/')
        sourcePath = sys.argv[2].replace('\\', '/')
        distPath = sys.argv[3].replace('\\', '/')
        CollectConvertInfo()
