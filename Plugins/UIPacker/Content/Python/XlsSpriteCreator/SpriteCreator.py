#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
import sys
import os
# import xlrd
import json
import shutil
import Sprite
import copy

def ToRect(strV) :
    coords = strV.split(";")
    if len(coords) != 4 :
        return None
    return Sprite.Rect(int(coords[0]), int(coords[1]), int(coords[2]), int(coords[3]))
    
def ToVec2(strV) :
    coords = strV.split(";")
    if len(coords) != 2 :
        return None
    return Sprite.Vec2(int(coords[0]), int(coords[1]))
    

# 解析每一个Sprite
# @return Sprite
def CreateSprite(spriteName, argStr, dbgMsg):
    argList = argStr.split("+")
    if len(argList) == 0 :
        print('sprite name:', spriteName, 'argStr:', argStr, 'not right!', dbgMsg)
        return None
    
    sprite = Sprite.Sprite(spriteName)
    # coords = argList[0].split(";")
    # if len(coords) != 4 :
    #     print('sprite name:', spriteName, 'coordinates:', argList[0], 'not right!', dbgMsg)
    #     return None
    
    rect = ToRect(argList[0])
    if rect is None :
        print('sprite name:', spriteName, 'coordinates:', argList[0], 'not right!', dbgMsg)
        return None
    # 解析坐标
    sprite.SetFrame(rect)
    return sprite
    
    
def CreateAtlas(startCol, titles, spriteNames, rowValues, dbgMsg) :
    atlasName = rowValues[0]
    if atlasName == "" or atlasName is None :
        return None
        
    atlasName = str(int(atlasName))
        
    atlas = Sprite.Atlas(atlasName)
    colSize = len(rowValues)
    print("CreateAtlas, colSize:", colSize)
    for j in range(0, colSize):
        colV = rowValues[j]
        if colV == "" :
            pass
        
        if titles[j] == u"原始图大小":
            size = ToVec2(colV)
            if size is not None :
                atlas.size.CopyFrom(size)
            pass
        
        
        if j >= startCol :    
            spriteName = spriteNames[j]
            if spriteName == "" or spriteName is None :
                continue
                
            sprite = CreateSprite(spriteName, colV, dbgMsg)
            if sprite is None :
                raise Exception("create sprite failed!", dbgMsg)
            print(">"+sprite.ToString())
            atlas.AddSprite(sprite)
            pass
    pass
    return atlas
    
#     "meta": {
# 	"app": "http://www.texturepacker.com",
# 	"version": "1.0",
# 	"image": "aa.png",
# 	"format": "RGBA8888",
# 	"size": {"w":2048,"h":2048},
# 	"scale": "1",
# 	"smartupdate": "$TexturePacker:SmartUpdate:1c2ae9cb59bb41481ed683add27e7e92$"
# }

def WriteAtlas(atlas, outputDir) :
    meta = { "app": "https://www.codeandweb.com/texturepacker", 'version': '1.0', "target": "paper2d", 'format': 'RGBA8888', 'scale': '1' }
    meta['image'] = Sprite.Atlas.PngFileName
    meta['size'] = atlas.size.ToSizeJsonObject()
    
    frames_json = {}
    for spr in atlas.frames :
        print("atlas.frames:" + spr.name)
        # print("WriteAtlas, atlasname:", atlas.name, " sprname:", spr.name, "==>", spr.ToJsonObject(), "++++", spr.ToString())
        frames_json[spr.name] = spr.ToJsonObject()
        pass
        
    atlasObj = { 'frames':frames_json, 'meta':meta }
    
    try:
        print("atlas.jsonfilepath:", atlas.jsonfilepath)
        atlasJsonfilepath = os.path.join(outputDir, atlas.jsonfilepath)
        if (not os.path.exists(os.path.dirname(atlasJsonfilepath))):
            os.makedirs(os.path.dirname(atlasJsonfilepath), 0o755)
        print('open...')
        print(atlasJsonfilepath)
        with open(atlasJsonfilepath, "w+") as jsonFile :
            dataStr = json.dumps(atlasObj, indent=4)
            print('dataStr:' + dataStr)
            jsonFile.seek(0)
            jsonFile.write(dataStr)
            jsonFile.close()
            pass
        
        atlasPngfilepath = os.path.join(outputDir, atlas.pngfilepath)
        print('atlasPngfilepath:' + atlasPngfilepath)
        print('shutil.copyfile:' + atlas.srcPngfilepath + ' to ' + atlasPngfilepath)
        shutil.copyfile(atlas.srcPngfilepath, atlasPngfilepath)
    except Exception as ex:
        print(ex)
        pass
    finally:
        pass
    pass
    

# def XlsToJson(xlsFilePath, outputDir):
#     data = xlrd.open_workbook(xlsFilePath)
#     sheets = data.sheets()
#
#     if len(sheets) == 0 :
#         print("xls里至少需要一张数据表")
#         pass
#
#     table = sheets[0]
#     titles = table.row_values(0)
#     names = table.row_values(1)
#
#
#     startCol = 0
#     startRow = 4
#     i = 0
#     for title in titles:
#         # print(">=", title)
#         i = i+1
#         if title == u"sprite开始":
#             print("sprite开始")
#             startCol = i
#             pass
#         pass
#
#     atlasList = []
#     # parse atlas
#     print("rows===>", table.nrows, table.ncols)
#     for i in range(table.nrows):
#         if i < startRow :
#             continue
#
#         dbgMsg = "@(xlsFilePath:"+xlsFilePath+", row:"+str(i+1)+")"
#         rowValues = table.row_values(i)
#         atlas = CreateAtlas(startCol, titles, names, rowValues, dbgMsg)
#         if atlas is None :
#             continue
#         atlas.SetSrcPngfilepath(os.path.dirname(xlsFilePath)+'/'+atlas.name+'.png')
#         print("atlas.SetSrcPngfilepath:" + os.path.dirname(xlsFilePath) + '/'+atlas.name+'.png')
#         atlasList.append(atlas)
#         pass
#     print("atlasList:" + str(len(atlasList)))
#     for atlas in atlasList:
#         WriteAtlas(atlas, outputDir)
#         pass
#     pass
#
# def XlsToJsonV1(xlsFilePath, jsonFileDir):
#     jsonFilePath=jsonFileDir + "/atlas.paper2dsprites"
#     data = xlrd.open_workbook(xlsFilePath)
#     sheets = data.sheets()
#
#     if len(sheets) == 0 :
#         print("xls里至少需要一张数据表")
#         pass
#
#     table = sheets[0]
#     titles = table.row_values(0)
#     names = table.row_values(1)
#
#     startCol = 2
#     startRow = 2
#
#     if (not os.path.exists(os.path.dirname(jsonFilePath))):
#         print("[XlsToJsonV1] json file not exist, path = " + jsonFilePath)
#         pass
#
#     print("open json path = " + jsonFilePath)
#     jsonDic={}
#     with open(jsonFilePath, "r") as jsonFile :
#         jsonDic = json.load(jsonFile)
#         #print(jsonDic)
#         #jsonFile.seek(0)
#         #jsonFile.write(dataStr.encode("utf-8"))
#         jsonFile.close()
#         pass
#
#     jsonFrames = jsonDic['frames']
#
#     for i in range(table.nrows):
#         if i < startRow :
#             continue
#         rowValues = table.row_values(i)
#
#         spriteName = rowValues[0]
#         if isinstance(rowValues[0],float):
#             spriteName = str(int(rowValues[0]))
#
#         print("process sprite name = " + spriteName)
#         #if jsonFrames.has_key(spriteName): #python2.7
#         if spriteName in jsonFrames:
#             j = -1
#             for name in names :
#                 j = j + 1
#                 if j < startCol:
#                     continue
#                 print("---------------------------")
#                 print("rectConfig config = " + rowValues[j])
#                 rectConfig = ToRect(rowValues[j])
#                 newObj = copy.deepcopy(jsonFrames[spriteName])
#                 print("copy new obj = " + str(newObj))
#                 jsonFrames[spriteName + name] = newObj
#                 frame = newObj['frame']
#                 print("frame = " + str(frame))
#                 frame['x'] = frame['x'] + rectConfig.x
#                 frame['y'] = frame['y'] + rectConfig.y
#                 frame['w'] = rectConfig.w
#                 frame['h'] =  rectConfig.h
#                 srcFrame = newObj['spriteSourceSize']
#                 srcFrame['w'] = rectConfig.w
#                 srcFrame['h'] = rectConfig.h
#                 srcSize = newObj['sourceSize']
#                 srcSize['w'] = rectConfig.w
#                 srcSize['h'] = rectConfig.h
#                 print("result new obj = " + str(newObj))
#                 print("---------------------------")
#
#     #print(jsonDic) #Python2.7
#     print(str(jsonDic))
#     #测试出新文件
#     #newFileName = os.path.splitext(jsonFilePath)[0]+ "_v1." + os.path.splitext(jsonFilePath)[1]
#     #正式走覆盖
#     newFileName=jsonFilePath
#     with open(newFileName, "w+") as newJsonFile:
#         newJsonFile.write(json.dumps(jsonDic,indent=1))
#         newJsonFile.close()
#         print("process json success, json file name = " + newFileName)
#         pass
#
    

# if __name__=='__main__':
    #xlsPath = "D:/Red2020/Project/test/portrait/sprites.xls"
    #outputPath = "D:/Red2020/Project/test/output"
    #XlsToJson(xlsPath, outputPath)

    # xlsPath = "G:/Svn/KDD_proj/trunk/KDDApp/Content/DesignResources/Textures/Icons/Profile/Portrait/sprites.xls"
    # jsonPath = "G:/icons"
    # XlsToJson(xlsPath, jsonPath)