#include "KDDWidget.h"

UKDDWidget::UKDDWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bEnableAnimation(true)
    , AutoShowAnim(nullptr)
    , AutoHideAnim(nullptr)
{
}

FString UKDDWidget::GetModuleName_Implementation() const
{
    return TEXT("");
}

// ======== 生命周期（自动播放动画） ========

void UKDDWidget::NativeConstruct()
{
    CacheAutoAnimations();
    Super::NativeConstruct();  // 触发 OnCreate → OnOpen（OnOpen 中自动播放 ShowAnim）
}

void UKDDWidget::NativeDestruct()
{
    // 先走 OnClose（自动播放 HideAnim），再走 OnDestroy
    Super::NativeDestruct();
}

void UKDDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}

// ======== 生命周期覆盖（注入动画） ========

void UKDDWidget::OnOpen()
{
    Super::OnOpen();
    PlayShowAnimation();
}

void UKDDWidget::OnClose()
{
    PlayHideAnimation();
    Super::OnClose();
}

// ======== 动画缓存 ========

void UKDDWidget::CacheAutoAnimations()
{
    AutoShowAnim = FindAnimationByName(TEXT("ShowAnim"));
    AutoHideAnim = FindAnimationByName(TEXT("HideAnim"));

    if (AutoShowAnim)
    {
        UE_LOG(LogTemp, Log, TEXT("[UKDDWidget] Auto-detected ShowAnim: %s"), *AutoShowAnim->GetName());
    }
    if (AutoHideAnim)
    {
        UE_LOG(LogTemp, Log, TEXT("[UKDDWidget] Auto-detected HideAnim: %s"), *AutoHideAnim->GetName());
    }
}

// ======== GetShowAnimation / GetHideAnimation 覆盖 ========

UWidgetAnimation* UKDDWidget::GetShowAnimation_Implementation() const
{
    // 优先自动识别的 ShowAnim
    if (AutoShowAnim) return AutoShowAnim;
    // 回退到自定义动画名
    if (!CustomShowAnimName.IsEmpty())
    {
        return const_cast<UKDDWidget*>(this)->FindAnimationByName(CustomShowAnimName);
    }
    return nullptr;
}

UWidgetAnimation* UKDDWidget::GetHideAnimation_Implementation() const
{
    // 优先自动识别的 HideAnim
    if (AutoHideAnim) return AutoHideAnim;
    // 回退到自定义动画名
    if (!CustomHideAnimName.IsEmpty())
    {
        return const_cast<UKDDWidget*>(this)->FindAnimationByName(CustomHideAnimName);
    }
    return nullptr;
}

// ======== 动画播放 ========

void UKDDWidget::PlayShowAnimation()
{
    if (!bEnableAnimation) return;

    UWidgetAnimation* Anim = GetShowAnimation();
    if (Anim && IsValid(this))
    {
        PlayAnimation(Anim);
        UE_LOG(LogTemp, Log, TEXT("[UKDDWidget] Playing Show animation: %s"), *Anim->GetName());
    }
}

void UKDDWidget::PlayHideAnimation()
{
    if (!bEnableAnimation) return;

    UWidgetAnimation* Anim = GetHideAnimation();
    if (Anim && IsValid(this))
    {
        PlayAnimation(Anim);
        UE_LOG(LogTemp, Log, TEXT("[UKDDWidget] Playing Hide animation: %s"), *Anim->GetName());
    }
}

void UKDDWidget::PlayLoopAnimation()
{
    if (!bEnableAnimation) return;
    if (CustomLoopAnimName.IsEmpty()) return;

    UWidgetAnimation* Anim = FindAnimationByName(CustomLoopAnimName);
    if (Anim && IsValid(this))
    {
        PlayAnimation(Anim, 0.0f, 0);  // 0 = 无限循环
        UE_LOG(LogTemp, Log, TEXT("[UKDDWidget] Playing Loop animation: %s"), *Anim->GetName());
    }
}

void UKDDWidget::StopLoopAnimation()
{
    if (CustomLoopAnimName.IsEmpty()) return;

    UWidgetAnimation* Anim = FindAnimationByName(CustomLoopAnimName);
    if (Anim && IsValid(this))
    {
        StopAnimation(Anim);
        UE_LOG(LogTemp, Log, TEXT("[UKDDWidget] Stopped Loop animation: %s"), *Anim->GetName());
    }
}

// ======== 编辑器测试按钮 ========

#if WITH_EDITOR
void UKDDWidget::Editor_PlayShowAnim()
{
    PlayShowAnimation();
}

void UKDDWidget::Editor_PlayLoopAnim()
{
    PlayLoopAnimation();
}

void UKDDWidget::Editor_PlayHideAnim()
{
    PlayHideAnimation();
}
#endif
