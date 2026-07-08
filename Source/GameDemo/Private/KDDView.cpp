#include "KDDView.h"

UKDDView::UKDDView(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

FString UKDDView::GetModuleName_Implementation() const
{
    // ModuleLocator 会使用 KDDBindingManager 的配表查找 Lua 路径
    return TEXT("");
}

void UKDDView::NativeConstruct()
{
    Super::NativeConstruct();
}

void UKDDView::NativeDestruct()
{
    Super::NativeDestruct();
}

void UKDDView::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}
