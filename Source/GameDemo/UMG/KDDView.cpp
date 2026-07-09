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

UWidgetAnimation* UKDDView::GetShowAnimation_Implementation() const
{
    return nullptr;
}

UWidgetAnimation* UKDDView::GetHideAnimation_Implementation() const
{
    return nullptr;
}

void UKDDView::NativeConstruct()
{
    Super::NativeConstruct();
    OnCreate();
    OnOpen();
}

void UKDDView::NativeDestruct()
{
    OnClose();
    OnDestroy();
    Super::NativeDestruct();
}

void UKDDView::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}

void UKDDView::OnCreate()
{
    UE_LOG(LogTemp, Log, TEXT("[UKDDView] OnCreate"));
}

void UKDDView::OnOpen()
{
    UE_LOG(LogTemp, Log, TEXT("[UKDDView] OnOpen"));
}

void UKDDView::OnClose()
{
    UE_LOG(LogTemp, Log, TEXT("[UKDDView] OnClose"));
}

void UKDDView::OnRefresh()
{
    UE_LOG(LogTemp, Log, TEXT("[UKDDView] OnRefresh"));
}

void UKDDView::OnDestroy()
{
    UE_LOG(LogTemp, Log, TEXT("[UKDDView] OnDestroy"));
}

UWidgetAnimation* UKDDView::FindAnimationByName(const FString& AnimName) const
{
    if (AnimName.IsEmpty()) return nullptr;

    // UMG 蓝图将动画存储在 WidgetTree 的 AllAnimations 数组中
    // 通过反射查找同名 FObjectProperty
    FProperty* Prop = GetClass()->FindPropertyByName(*AnimName);
    if (Prop)
    {
        FObjectProperty* ObjProp = CastField<FObjectProperty>(Prop);
        if (ObjProp)
        {
            return Cast<UWidgetAnimation>(ObjProp->GetObjectPropertyValue_InContainer(this));
        }
    }
    return nullptr;
}
