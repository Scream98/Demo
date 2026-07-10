#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
import os

class Vec2:
    def __init__(self, x, y) :
        self.Set(x, y)
        pass
    def Set(self, x, y) :
        self.x = x
        self.y = y
        pass
    def CopyFrom(self, vec2) :
        self.x = vec2.x
        self.y = vec2.y
        pass
    def ToJsonObject(self) :
        return self.__dict__
        
    def ToSizeJsonObject(self) :
        return { 'w':self.x, 'h':self.y }

class Rect:
    def __init__(self, x, y, w, h) :
        self.Set(x, y, w, h)
        pass
    def Set(self, x, y, w, h) :
        self.x = x
        self.y = y
        self.w = w
        self.h = h
        pass
    
    def SetPosition(self, posVec2) :
        self.x = posVec2.x
        self.y = posVec2.y
        pass
        
    def SetSize(self, sizeVec2) :
        self.w = sizeVec2.x
        self.h = sizeVec2.y
        pass
        
    def SetSize2(self, w, h) :
        self.w = w
        self.h = h
        pass
        
    def CopyFrom(self, rect) :
        self.x = rect.x
        self.y = rect.y
        self.w = rect.w
        self.h = rect.h
        pass
    
    def ToJsonObject(self) :
        return self.__dict__
        
    def ToString(self) :
        return "Rect("+str(self.x)+","+str(self.y)+","+str(self.w)+","+str(self.h)+")"  
        

class Sprite:
    def __init__(self, name) :
        self.mirror = False  # 是否镜像
        self.name = name
        self.frame = Rect(0, 0, 0, 0)
        self.rotated = False
        self.trimmed = False
        self.spriteSourceSize = Rect(0, 0, 0, 0)
        self.sourceSize = Vec2(0, 0)
        pass

    def CopyFrom(self, other) :
        self.mirror = other.mirror
        self.name = other.name
        self.frame.CopyFrom(other.frame)
        self.rotated = other.rotated
        self.trimmed = other.trimmed
        self.spriteSourceSize.CopyFrom(other.spriteSourceSize)
        self.sourceSize.CopyFrom(other.sourceSize)
        
    def SetFrame(self, frameRect):
        self.frame.CopyFrom(frameRect)
        self.spriteSourceSize.SetSize2(frameRect.w, frameRect.h)
        self.sourceSize.Set(frameRect.w, frameRect.h) 
        pass
    
    def ToString(self) :
        return "Sprite("+self.name+","+self.frame.ToString()+")"
        
    def ToJsonObject(self) :
        obj = self.__dict__.copy()
        obj['frame'] = self.frame.ToJsonObject()
        obj['spriteSourceSize'] = self.spriteSourceSize.ToJsonObject()
        obj['sourceSize'] = self.sourceSize.ToSizeJsonObject()
        return obj
    
    
class Atlas:
    PngFileName = "atlasTex.png"
    JsonFileName = "atlas.paper2dsprites"
    
    def __init__(self, name):
        self.name = name
        print("[Atlas]===name:", name)
        self.pngfilepath = name + '/' + Atlas.PngFileName   # 目标png路径
        self.jsonfilepath = name + '/' + Atlas.JsonFileName 
        self.srcPngfilepath = ""  # 源png路径
        self.format = ""
        self.size = Vec2(0, 0)
        self.scale = 1
        self.frames = []
        pass
        
    def SetSrcPngfilepath(self, srcPngfilepath):
        self.srcPngfilepath = srcPngfilepath
        pass
        
    def AddSprite(self, sprite):
        self.frames.append(sprite)
        pass
        
    # def ToJsonObject(self) :
    #     obj = self.__dict__.copy()
    #     obj['size'] = self.size.ToSizeJsonObject()
    #     frames_json = {}
    #     for spr in self.frames :
    #         frames_json[spr.name] = spr.ToJsonObject()
    #         pass
    #     obj['frames'] = frames_json
    #     return obj