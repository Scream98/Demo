#include "KDDBindingManager.h"

TMap<FString, FString> UKDDBindingManager::BindingConfig;

void UKDDBindingManager::RegisterBinding(const FString& ClassName, const FString& LuaPath)
{
    if (ClassName.IsEmpty() || LuaPath.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[KDDBindingManager] RegisterBinding: invalid params - ClassName=%s LuaPath=%s"),
               *ClassName, *LuaPath);
        return;
    }

    BindingConfig.FindOrAdd(ClassName) = LuaPath;
    UE_LOG(LogTemp, Log, TEXT("[KDDBindingManager] RegisterBinding: %s -> %s"), *ClassName, *LuaPath);
}

FString UKDDBindingManager::GetLuaPath(const FString& ClassName)
{
    const FString* Found = BindingConfig.Find(ClassName);
    if (Found)
    {
        return *Found;
    }

    // 兜底：去掉 _C 后缀再试一次
    if (ClassName.EndsWith(TEXT("_C")))
    {
        const FString AltName = ClassName.LeftChop(2);
        Found = BindingConfig.Find(AltName);
        if (Found)
        {
            return *Found;
        }
    }

    return TEXT("");
}

const TMap<FString, FString>& UKDDBindingManager::GetBindingConfig()
{
    return BindingConfig;
}
