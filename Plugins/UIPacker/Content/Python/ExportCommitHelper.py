#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# 这个脚本主要有以下用途
# 1. 调用commandline导出sprite的UV信息，结合diff.json做差异化校验
# 2. svn全增量，删除，modify 差异化提交(实在是批处理处理json太麻烦了...，)

import os
import sys
import json
import subprocess
import re

OPTION_EXPORT_ICON_SETTING_BEFORE_GEN = 'esb'
OPTION_EXPORT_ICON_SETTING_AFTER_GEN = 'esa'
OPTION_EXPORT_ICON_SVN_COMMIT = 'commit'
OPTION_EXPORT_ICON_CLEAN_INCREMENTAL = 'clean'

SVN_STATE_MISSING = "!"
SVN_STATE_UNVERSION = "?"

# 存档路径
spriteSettingsInfoBeforeGenFileName = "spriteSettingsInfoBeforeGen.json"
spriteSettingsInfoAfterGenFileName = "spriteSettingsInfoAfterGen.json"
# 用来查询定位动态路径
pathSearchkey = "Content"

# 工作区变量
workDir = ""
isExportAll = False

supportResType = {
    "sprite": "sprite",
    "texture": "texture"
}

# 遍历Target目录的时候额外加的Token，比如图片就追加一个Frames目录
walkFolderPathAddToken = ""
walkFolderPathAddTokenList = {
    "sprite": "Frames",
    "texture": ""
}

def osSystem(cmd, ignoreError=False):
    os.system(("echo " + cmd))
    subprocess.call(cmd)
    try:
        out_bytes = subprocess.check_output(cmd)
        out_text = out_bytes.decode('gbk')
    except subprocess.CalledProcessError as e:
        errMsg = e.output.decode('gbk')
        if not ignoreError:
            os.system("echo " + errMsg)
            sys.exit(-1)
        else:
            out_text = errMsg
    return out_text



def cleanIncrementalEnv():
    # 清理掉增量存档，那么这次更新就是走全量更新了
    cleanSpriteSettingsInfo()
    diffInfoPath = os.path.join(workDir, "diff.json")
    if os.path.exists(diffInfoPath):
        os.remove(diffInfoPath)
    archiveInfoPath = os.path.join(workDir, "exportArchive.json")
    if os.path.exists(archiveInfoPath):
        os.remove(archiveInfoPath)


def cleanSpriteSettingsInfo():
    outPath = os.path.join(workDir, spriteSettingsInfoBeforeGenFileName)
    if os.path.exists(outPath):
        os.remove(outPath)
    outPath = os.path.join(workDir, spriteSettingsInfoAfterGenFileName)
    if os.path.exists(outPath):
        os.remove(outPath)


def exportSpriteSettingsInfo(args, configOutSavePath):
    outPath = os.path.join(workDir, configOutSavePath)
    if os.path.exists(outPath):
        os.remove(outPath)
    if isExportAll == True:
        return

    # 如果isExportAll 为 Flase，diff文件一定存在
    diffInfoPath = os.path.join(workDir, "diff.json")
    with open(diffInfoPath, 'r') as read_f:
        diffInfo = json.load(read_f)
    diffPath = ""
    for unit in diffInfo['diffFolder']:
        if diffPath != "" and len(unit['target'] or "") > 0:
            diffPath += "$"
        diffPath += unit['target']
    # 合并为一次调用commandlet
    if len(diffPath) > 0:
        bRet = osSystem(f"{args[2]} {args[3]} -skipcompile -run=ExportSpriteUVInfo config=\"{outPath}\" path=\"{diffPath}\" bclean=\"1\" -NullRHI")
        print("exportSpriteSettingsInfo success, save info to: %s" % outPath)


def judgeAndCommitChanges(args):
    # target root 目录执行一次svn add
    rootPath = os.path.abspath(args[2]).replace("\\", "/")
    osSystem(f"svn add {rootPath} --force")
    if isExportAll == True:
        # 删除所有 Missing 文件
        retInfo = osSystem(f'svn st {rootPath}') or ""
        infoList = retInfo.splitlines()
        for info in infoList:
            svnOpState, pathInfo = info[0:1].strip(), info[2:].strip().replace("\\", "/")
            if svnOpState == SVN_STATE_MISSING:
                osSystem(f"svn delete {pathInfo} --force")
    else:
        modifyInfo = {}
        diffInfoPath = os.path.join(workDir, "diff.json")
        if os.path.exists(diffInfoPath):
            with open(diffInfoPath, 'r') as read_f:
                diffInfo = json.load(read_f)
                for unit in diffInfo['diffFolder']:
                    target = os.path.abspath(unit['target'] + (('/' + walkFolderPathAddToken) if len(walkFolderPathAddToken) > 0 else ''))
                    modifyInfo.update({target: unit['changelist']})
        diffUVSettings = {}
        beforeInfoPath = os.path.join(workDir, spriteSettingsInfoBeforeGenFileName)
        if os.path.exists(beforeInfoPath):
            with open(beforeInfoPath, 'r') as read_f:
                beforeInfo = json.load(read_f)
                for unit in beforeInfo['SpriteSettings']:
                    diffUVSettings.update({unit['fullpath']: { 'uv_x': unit['uv_x'], 'uv_y': unit['uv_y'], 'ismodify': False }})
        afterInfoPath = os.path.join(workDir, spriteSettingsInfoAfterGenFileName)
        if os.path.exists(afterInfoPath):
            with open(afterInfoPath) as read_f:
                afterInfo = json.load(read_f)
                for unit in afterInfo['SpriteSettings']:
                    if unit['fullpath'] in diffUVSettings.keys():
                        if unit['uv_x'] != diffUVSettings[unit['fullpath']]['uv_x'] or unit['uv_y'] != diffUVSettings[unit['fullpath']]['uv_y']:
                            diffUVSettings[unit['fullpath']]['ismodify'] = True

        for path, list in modifyInfo.items():
            for root, dirs, files in os.walk(path):
                for name in files:
                    nameSplit = os.path.splitext(name)
                    fullPath = os.path.join(root, name).replace("\\", "/")
                    if nameSplit[0] != 'atlas' and nameSplit[1] == '.uasset':
                        # 不在svn diff列表里，看看是不是实际上没有发生变化的，是的话 revert
                        if not nameSplit[0] in list:
                            if not bool(diffUVSettings):
                                continue
                            searchPath = fullPath
                            index = fullPath.find(pathSearchkey)
                            if index > 0:
                                searchPath = "/Game/" + fullPath[index + 8].replace(".uasset", "")
                            if searchPath in diffUVSettings.keys():
                                if not diffUVSettings[searchPath]['ismodify']:
                                    osSystem(f"svn revert -R {fullPath}")
                        else:
                            # 标记下已经遍历了，剩下的没有遍历到的，就是diff里面删除的
                            list[nameSplit[0]] = False


            for file, state in list.items():
                # 遍历完了，还没有走到的，已经 missing，直接删除, 注意这儿是UE资源加上 .uasset
                if state:
                    # 往上一级先看看父级目录还在不在，不在了，可以直接把父级目录删了，否则删除自己
                    delFile = os.path.abspath(os.path.join(path, file + '.uasset')).replace("\\", "/")
                    checkDir = delFile
                    isDelParent = False
                    isParttenFind = True
                    while isParttenFind:
                        replaceIdx = checkDir.rfind("/")
                        isParttenFind = replaceIdx > 0
                        if isParttenFind:
                            checkDir = checkDir[:replaceIdx]
                            if checkDir.endswith("/" + walkFolderPathAddToken):
                                checkDir = checkDir[:len(checkDir) - len("/" + walkFolderPathAddToken)]
                            if not os.path.exists(checkDir):
                                deleteWithInVersionCheck(checkDir)
                                isDelParent = True
                                break
                    if not isDelParent:
                        deleteWithInVersionCheck(delFile)
        # Root目录 add 过了，里面直接不管
        # for unit in diffInfo['addFolder']:
        #     target = os.path.abspath(unit['target'])
        #     osSystem(f"svn add {target} --force")
        for unit in diffInfo['delFolder']:
            target = os.path.abspath(unit['target'])
            deleteWithInVersionCheck(target)

    # missing的不管就行了，不会提交，只会提交变化部分，防止暴力删除的其他目录被同步，下次更新采取revert - update 策略即可
    comment = "ExportIcons"
    if len(args) > 7:
        comment = args[7] or "ExportIcons"
    osSystem(f"svn commit {rootPath} -m \"[{args[3]}]Commit {comment}, {args[5]} --id={args[4]} --committer={args[3]}\" --username a1_red --password A1Red@dev --no-auth-cache")


def deleteWithInVersionCheck(target):
    # 有可能目标文件被手动删过了，不在版本控制里，这时候就不用删了
    delFolderCheckInfo = osSystem(f'svn info {target}', True)
    delFolderCheckInfoList = delFolderCheckInfo.splitlines()
    isInVersion = False
    for delFolderInfo in delFolderCheckInfoList:
        svnErrMatch = re.search(r'Revision: \d+', delFolderInfo)
        if svnErrMatch:
            isInVersion = True
            break
    if isInVersion:
        osSystem(f"svn delete {target} --force")
    else:
        osSystem(f"svn revert -R {target}")
        if os.path.exists(target):
            if os.path.isfile(target):
                os.remove(target)
            elif os.path.isdir(target):
                os.rmdir(target)
        print("svn delete skiped, target: \'%s\' not in version control" % target)



if __name__=='__main__':
    # 模拟数据
    # sys.argv = ["", "", "", ""]
    # sys.argv[0] = "D:/Company/Red/EmptyProj/Plugins/UIPacker/Content/Python/ExportCommitHelper.py"
    # sys.argv[1] = "esb"
    # sys.argv[2] = "D:/EpicGames/Engines/RedSourceEngine/Engine/Binaries/Win64/UE4Editor-Cmd.exe"
    # sys.argv[3] = "D:/Company/Red/EmptyProj/EmptyProj.uproject"

    if len(sys.argv) < 2:
        print("parameters not enough")
        sys.exit(-1)
    workDir = os.path.abspath(os.path.dirname(sys.argv[0]))
    diffInfoPath = os.path.join(workDir, "diff.json")
    if os.path.exists(diffInfoPath):
        with open(diffInfoPath, 'r') as read_f:
            diffInfo = json.load(read_f)
            isExportAll = diffInfo['isExportAll']
    else:
        isExportAll = True

    optionType = sys.argv[1]
    if optionType == OPTION_EXPORT_ICON_SETTING_BEFORE_GEN:
        # arg[1] command, arg[2] editor exe path, arg[3] uproject path
        exportSpriteSettingsInfo(sys.argv, spriteSettingsInfoBeforeGenFileName)
    elif optionType == OPTION_EXPORT_ICON_SETTING_AFTER_GEN:
        # arg[1] command, arg[2] editor exe path, arg[3] uproject path
        exportSpriteSettingsInfo(sys.argv, spriteSettingsInfoAfterGenFileName)
    elif optionType == OPTION_EXPORT_ICON_SVN_COMMIT:
        # arg[1] command, arg[2] target root folder, arg[3] BK_CI_START_USER_NAME, arg[4] BK_CI_PIPELINE_ID, arg[5] commitInfo, arg[6] type, arg[7] comment
        resType = sys.argv[6] or ""
        walkFolderPathAddToken = walkFolderPathAddTokenList[resType] or ""
        if resType == supportResType['sprite']:
            judgeAndCommitChanges(sys.argv)
        elif resType == supportResType['texture']:
            cleanSpriteSettingsInfo()
            judgeAndCommitChanges(sys.argv)
    elif optionType == OPTION_EXPORT_ICON_CLEAN_INCREMENTAL:
        # arg[1] command
        cleanIncrementalEnv()

