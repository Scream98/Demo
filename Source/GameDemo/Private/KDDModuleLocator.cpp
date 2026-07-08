#include "KDDModuleLocator.h"
#include "KDDBindingManager.h"

FString UKDDModuleLocator::Locate(const UObject* Object)
{
    if (!Object || !Object->GetClass())
    {
        return TEXT("");
    }

    const FString ClassName = Object->GetClass()->GetName();
    return UKDDBindingManager::GetLuaPath(ClassName);
}
