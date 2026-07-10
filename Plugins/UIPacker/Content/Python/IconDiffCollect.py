#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import json
import subprocess
import re

iconResPath = ""
# 如果没有查询到有效的svn高度值，直接全量更新
# isExportAll = True
exportDiffInfoPath = "./diff.json"
exportArchiveFullJsonPath = "./exportArchive.json"
diffInfo = {}

keyW = "dyn"
pathToken = "/Game/DesignResources/Textures/"
sourceRootPath = "../../../../Content/DesignResources/Textures/"

targetRootPath = ''
targetPathToken = ''
targetDynamicRootPath = ''
targetDynamicPathToken = ''

# 参数6(相对targetRootPath的输出目录指定别名目录)传递后，计算出来的Replace key，用于计算target目录时进行替换
targetDynamicAliasReplaceToken = ["", ""]

targetRootPathMap = {
	'sprite': '../../../../Content/KDDApp/Base/UI/SpriteSheet/',
	'texture': '../../../../Content/KDDApp/Base/UI/Textures/'
}
targetPathTokenMap = {
	'sprite': 'KDDApp/Base/UI/SpriteSheet',
	'texture': 'KDDApp/Base/UI/Textures'
}
targetDynamicRootPathMap = {
	'sprite': '../../../../Content/KDDApp/Base/UI/DynamicLoadRes/SpriteSheet/',
	'texture': '../../../../Content/KDDApp/Base/UI/DynamicLoadRes/Textures/'
}
targetDynamicPathTokenMap = {
	'sprite': 'KDDApp/Base/UI/DynamicLoadRes/SpriteSheet',
	'texture': 'KDDApp/Base/UI/DynamicLoadRes/Textures'
}

isDynamic = False
archiveKey = ""
exportReplacePath = []

OPTION_EXPORT = "e"
OPTION_ARCHIVE = 'a'

MODIFY = "M"
ADD = "A"
DELETE = "D"


def osSystem(cmd):
    os.system(("echo " + cmd))
    # subprocess.call(cmd)
    try:
        out_bytes = subprocess.check_output(cmd)
        out_text = out_bytes.decode('gbk')
    except subprocess.CalledProcessError as e:
        os.system("echo " + e.output.decode('gbk'))
        sys.exit(-1)
    return out_text


def simpleCheckIsFolder(path):
    if os.path.isdir(path):
        return True
    if os.path.isfile(path):
        return False
    lastSection = path.replace("\\", "/").split("/")[-1]
    sectionSplit = os.path.splitext(lastSection)
    if sectionSplit[1] == "":
        return True
    return False


def simpleCheckIsFile(path):
    if os.path.isfile(path):
        return True
    if os.path.isdir(path):
        return False
    lastSection = path.replace("\\", "/").split("/")[-1]
    sectionSplit = os.path.splitext(lastSection)
    if sectionSplit[1] != "":
        return True
    return False


def isParentFolderExit(checkFolder, checkDic):
    # checkDic的key是目录，主要是为了便于查找
    isParentFolderExit = False
    folderSubList = checkFolder.replace("\\", "/").split("/")
    checkDir = ""
    for subFolder in folderSubList:
        if checkDir != "":
            checkDir += "/"
        checkDir += subFolder
        if checkDir in checkDic.keys():
            isParentFolderExit = True
            break
    return isParentFolderExit


def tryRemoveSubFolderFromDic(checkFolder, checkDic):
    checkFolder = checkFolder.replace("\\", "/")
    for folder in list(checkDic.keys()):
        if folder.startswith(checkFolder):
            del checkDic[folder]


def getTargetFile(path):
    if not isDynamic:
        return path
    targetFile = path.split("_")
    if len(targetFile) < 2:
        return path
    if keyW in targetFile[len(targetFile)-1].lower():
        targetFile.pop()
    return "_".join(targetFile)


def convOutDiffInfo(sourcePath, changeList):
    convPath = sourcePath.replace(exportReplacePath[1], exportReplacePath[0])
    targetPath = getTargetFile(convPath.replace(sourceRootPath, targetRootPath, 1))
    if len(targetDynamicAliasReplaceToken[0]) > 0:
        targetPath = targetPath.replace(targetDynamicAliasReplaceToken[0], targetDynamicAliasReplaceToken[1])
    return { "source": convPath, "target": targetPath, "changelist": changeList or {} }


def archiveTmpSvnRevision(exportArchive, curRevision, archivePath):
    exportArchive[archiveKey].update({"tmpRevision": curRevision})
    # 写入当前操作的高度，全部成功后写入 lastSuccessRevision
    with open(archivePath, 'w') as write_f:
        json.dump(exportArchive, write_f, indent=4, ensure_ascii=False)


def exportDiffInfo(iconResPath, diffOutPath, archivePath):
    # 存档版本高度
    lastSuccessRevision = -1
    exportArchive = {}
    if os.path.exists(archivePath):
        with open(archivePath, 'r') as read_f:
            exportArchive = json.load(read_f)
            if archiveKey in exportArchive.keys():
                archiveInfo = exportArchive[archiveKey] or {}
                if 'lastSuccessRevision' in archiveInfo.keys():
                    lastSuccessRevision = archiveInfo['lastSuccessRevision'] or -1
    if not archiveKey in exportArchive.keys():
        exportArchive.update({archiveKey: {}})
    curRevision = -1
    retInfo = osSystem(f'svn info {iconResPath}')
    infoList = retInfo.splitlines()
    for info in infoList:
        if info.find('Revision:') >= 0:
            curRevision = int(re.findall("\d+", info)[0])
            print('curRevision: %d' % curRevision)
    if curRevision < 0:
        print("get svn revision info failed.")
        sys.exit(-1)
    if lastSuccessRevision > 0:
        if curRevision == lastSuccessRevision:
            archiveTmpSvnRevision(exportArchive, curRevision, archivePath)
            print("there is no update need to export.")
            sys.exit(1)
        else:
            print("svn archive revision valid, curRevision[%d], lastSuccessRevision[%d], use incremental export!" % (curRevision, lastSuccessRevision))
    else:
        print("svn archive revision error, curRevision[%d], lastSuccessRevision[%d], use full export!" % (curRevision, lastSuccessRevision))

    # 默认使用全量导出，能够拉到差异信息再使用差量导出
    diffInfo = {"isExportAll": True, "diffFolder": {}, "addFolder": {}, "delFolder": {}}
    if curRevision > lastSuccessRevision and lastSuccessRevision > 0:
        retInfo = osSystem(f'svn diff -r {lastSuccessRevision}:{curRevision} {iconResPath} --summarize')
        # print(retInfo)
        infoList = retInfo.splitlines()
        # 先收集一下状态是 Add 或者 Delete的文件夹，再具体判断文件的时候，位于这些文件夹的子目录就不管
        for info in infoList:
            svnOpState, pathInfo = info[0:1].strip(), info[2:].strip().replace("\\", "/")
            if simpleCheckIsFolder(pathInfo):
                if svnOpState == DELETE:
                    bRet = isParentFolderExit(pathInfo, diffInfo["delFolder"])
                    if not bRet:
                        tryRemoveSubFolderFromDic(pathInfo, diffInfo["delFolder"])
                        diffInfo["delFolder"].update({pathInfo: {}})
                elif svnOpState == ADD or svnOpState != MODIFY:
                    bRet = isParentFolderExit(pathInfo, diffInfo["addFolder"])
                    if not bRet:
                        tryRemoveSubFolderFromDic(pathInfo, diffInfo["addFolder"])
                        diffInfo["addFolder"].update({pathInfo: {}})
        # 按照文件遍历收集
        for info in retInfo.splitlines():
            svnOpState, pathInfo = info[0:1].strip(), info[2:].strip().replace("\\", "/")
            if simpleCheckIsFile(pathInfo):
                bRet = False
                if svnOpState == ADD:
                    bRet = isParentFolderExit(pathInfo, diffInfo["addFolder"])
                elif svnOpState == DELETE:
                    bRet = isParentFolderExit(pathInfo, diffInfo["delFolder"])
                if bRet:
                    continue
                (path, filename) = os.path.split(pathInfo)
                if not path in diffInfo["diffFolder"].keys():
                    diffInfo["diffFolder"].update({path: {}})
                diffInfo["diffFolder"][path].update({os.path.splitext(filename)[0]: True})
        diffInfo["isExportAll"] = False
    else:
        pass
        # archiveTmpSvnRevision(exportArchive, curRevision, archivePath)

    # 重新梳理下导出结构，便于其他地方使用
    convDiffInfo = {"isExportAll": diffInfo["isExportAll"], "diffFolder": [], "addFolder": [], "delFolder": []}
    for k, v in diffInfo["diffFolder"].items():
        convDiffInfo["diffFolder"].append(convOutDiffInfo(k, v))
    for k, v in diffInfo["addFolder"].items():
        convDiffInfo["addFolder"].append(convOutDiffInfo(k, v))
    for k, v in diffInfo["delFolder"].items():
        convDiffInfo["delFolder"].append(convOutDiffInfo(k, v))

    diffInfo = convDiffInfo

    with open(diffOutPath, 'w') as write_f:
        json.dump(diffInfo, write_f, indent=4, ensure_ascii=False)

    archiveTmpSvnRevision(exportArchive, curRevision, archivePath)
    # 不需要更新跳过
    if not diffInfo["isExportAll"] and not bool(diffInfo['diffFolder']) and not bool(diffInfo['addFolder']) and not bool(diffInfo['delFolder']):
        print("there is no update need to export.")
        sys.exit(1)


def archiveExportInfo(archivePath):
    if os.path.exists(archivePath):
        with open(archivePath, 'r') as read_f:
            exportArchive = json.load(read_f)
            archiveTarget = {"lastSuccessRevision": -1, "tmpRevision": -1}
            if archiveKey in exportArchive.keys():
               archiveTarget = exportArchive[archiveKey]
            archiveTarget["lastSuccessRevision"] = archiveTarget["tmpRevision"] or -1
            archiveTarget["tmpRevision"] = -1
    with open(archivePath, 'w') as write_f:
        json.dump(exportArchive, write_f, indent=4, ensure_ascii=False)


if __name__ == "__main__":
    # 资源总目录，子项目录
    # sys.argv = ["", "", "", "", "", "", ""]
    # sys.argv[0] = "D:/Company/Red/EmptyProj/Plugins/UIPacker/Content/Python/IconDiffCollect.py"
    # sys.argv[1] = "e"
    # sys.argv[2] = "texture" #sprite
    # sys.argv[3] = "/Game/t/Icons/Other/Tex" # /Game/t/Icons/HeadIcons
    # sys.argv[4] = "EmptyProj/Content/t/Icons"
    # sys.argv[5] = "true"
    # sys.argv[6] = "Other" # @[opt] 相对targetRootPath的输出目录指定别名目录，这样的话最终的目标目录不能直接映射过去了，需要用这个算
    if len(sys.argv) < 4:
        print("parameters not enough")
        sys.exit(-1)
    targetRootPath = targetRootPathMap[sys.argv[2]]
    targetPathToken = targetPathTokenMap[sys.argv[2]]
    targetDynamicRootPath = targetDynamicRootPathMap[sys.argv[2]]
    targetDynamicPathToken = targetDynamicPathTokenMap[sys.argv[2]]
    archiveKey = (sys.argv[3] or "").replace("\\", "/")
    optionType = sys.argv[1]
    dir = os.path.abspath(os.path.dirname(sys.argv[0]))
    if optionType == OPTION_EXPORT:
        if len(sys.argv) < 5:
            print("parameters not enough")
            sys.exit(-1)

        tmp_path = sys.argv[4].split("/")
        if len(tmp_path) > 0:
            if tmp_path[-1] == "":
                tmp_path = tmp_path[0:-2]
            else:
                tmp_path = tmp_path[0:-1]
        for i in range(len(tmp_path)):
            if tmp_path[i] == "Content":
                tmp_path = tmp_path[i:]
                replaceSourcePath = ["/Content/DesignResources/Textures/","/"+"/".join(tmp_path) + "/"]
                tmp_path[0] = "Game"
                replacePath = ["/Game/DesignResources/Textures/","/"+"/".join(tmp_path) + "/"]
                break
        print("[diffCollect]----【replaceSourcePath:" + ",".join(replaceSourcePath) + "】【replacePath:" + ",".join(replacePath) + "】")
        exportReplacePath = []
        exportReplacePath.append(dir + "\\")
        exportReplacePath.append((dir + "\\").replace("\\", "/"))
        sourceRootPath = os.path.join(dir,sourceRootPath).replace(replaceSourcePath[0],replaceSourcePath[1])
        selectFolder = sys.argv[3].replace(replacePath[0],replacePath[1])
        pathToken = pathToken.replace(replacePath[0],replacePath[1])

        #是否是动态路径
        isDynamic = False
        if len(sys.argv) > 5 and sys.argv[5] == "true":
            isDynamic = True
        targetRootPath = os.path.join(dir,targetRootPath)
        if isDynamic:
            targetRootPath = os.path.join(dir, targetDynamicRootPath)
            targetPathToken = targetDynamicPathToken
            print("[diffCollect]----【targetRootPath：" + targetRootPath + "】【targetPathToken：" + targetPathToken + "】")

        if len(sys.argv) > 6:
            targetDynamicAliasReplaceToken[0] = (targetPathToken + "/" + selectFolder.replace(pathToken, "")).replace("\\", "/")
            targetDynamicAliasReplaceToken[1] = targetPathToken + "/" + selectFolder.replace(pathToken, "").replace("\\", "/").split("/")[0] + "/" + sys.argv[6]

        exportDiffInfo(sourceRootPath + selectFolder.replace(pathToken, ""), os.path.join(dir, exportDiffInfoPath), os.path.join(dir, exportArchiveFullJsonPath))
    elif optionType == OPTION_ARCHIVE:
        archiveExportInfo(os.path.join(dir, exportArchiveFullJsonPath))
    else:
        print("unknown option type")
        sys.exit(-1)