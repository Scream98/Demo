#include "KDDWidget.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"

UKDDWidget::UKDDWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bEnableAnimation(true)
    , AutoShowAnim(nullptr)
    , AutoHideAnim(nullptr)
    , AutoLoopAnim(nullptr)
{
}

FString UKDDWidget::GetModuleName_Implementation() const
{
    // ModuleLocator 会使用 KDDBindingManager 的配表查找 Lua 路径
    return TEXT("");
}

// ======== 生命周期 ========

void UKDDWidget::NativeConstruct()
{
    CacheAutoAnimations();
    Super::NativeConstruct();
    OnCreate();
    OnOpen();
}

void UKDDWidget::NativeDestruct()
{
    OnClose();
    OnDestroy();
    Super::NativeDestruct();
}

void UKDDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}

// ======== 详细生命周期 ========

void UKDDWidget::OnCreate()
{
    UE_LOG(LogTemp, Log, TEXT("[UKDDWidget] OnCreate"));
}

void UKDDWidget::OnOpen()
{
    UE_LOG(LogTemp, Log, TEXT("[UKDDWidget] OnOpen"));
    PlayShowAnimation();
}

void UKDDWidget::OnClose()
{
    PlayHideAnimation();
    UE_LOG(LogTemp, Log, TEXT("[UKDDWidget] OnClose"));
}

void UKDDWidget::OnRefresh()
{
    UE_LOG(LogTemp, Log, TEXT("[UKDDWidget] OnRefresh"));
}

void UKDDWidget::OnDestroy()
{
    UE_LOG(LogTemp, Log, TEXT("[UKDDWidget] OnDestroy"));
}

// ======== 动画缓存 ========

void UKDDWidget::CacheAutoAnimations()
{
    AutoShowAnim = FindAnimationByName(TEXT("ShowAnim"));
    AutoHideAnim = FindAnimationByName(TEXT("HideAnim"));
    AutoLoopAnim = FindAnimationByName(TEXT("LoopAnim"));

    if (AutoShowAnim)
    {
        UE_LOG(LogTemp, Log, TEXT("[UKDDWidget] Auto-detected ShowAnim: %s"), *AutoShowAnim->GetName());
    }
    if (AutoHideAnim)
    {
        UE_LOG(LogTemp, Log, TEXT("[UKDDWidget] Auto-detected HideAnim: %s"), *AutoHideAnim->GetName());
    }
    if (AutoLoopAnim)
    {
        UE_LOG(LogTemp, Log, TEXT("[UKDDWidget] Auto-detected LoopAnim: %s"), *AutoLoopAnim->GetName());
    }
}

// ======== GetShowAnimation / GetHideAnimation ========

UWidgetAnimation* UKDDWidget::GetShowAnimation_Implementation() const
{
    // 优先自动识别的 ShowAnim
    if (AutoShowAnim) return AutoShowAnim;
    // 回退到自定义动画名
    if (!CustomShowAnimName.IsEmpty())
    {
        return const_cast<UKDDWidget*>(this)->FindAnimationByName(CustomShowAnimName);
    }
    // 兜底：直接按默认名查找
    return const_cast<UKDDWidget*>(this)->FindAnimationByName(TEXT("ShowAnim"));
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
    // 兜底：直接按默认名查找
    return const_cast<UKDDWidget*>(this)->FindAnimationByName(TEXT("HideAnim"));
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

UWidgetAnimation* UKDDWidget::GetLoopAnimation() const
{
    // 优先自动识别的 LoopAnim
    if (AutoLoopAnim) return AutoLoopAnim;
    // 回退到自定义动画名
    if (!CustomLoopAnimName.IsEmpty())
    {
        return const_cast<UKDDWidget*>(this)->FindAnimationByName(CustomLoopAnimName);
    }
    // 兜底：直接按默认名查找
    return const_cast<UKDDWidget*>(this)->FindAnimationByName(TEXT("LoopAnim"));
}

void UKDDWidget::PlayLoopAnimation()
{
    if (!bEnableAnimation) return;

    UWidgetAnimation* Anim = GetLoopAnimation();
    if (Anim && IsValid(this))
    {
        PlayAnimation(Anim, 0.0f, 0);  // 0 = 无限循环
        UE_LOG(LogTemp, Log, TEXT("[UKDDWidget] Playing Loop animation: %s"), *Anim->GetName());
    }
}

void UKDDWidget::StopLoopAnimation()
{
    UWidgetAnimation* Anim = GetLoopAnimation();
    if (Anim && IsValid(this))
    {
        StopAnimation(Anim);
        UE_LOG(LogTemp, Log, TEXT("[UKDDWidget] Stopped Loop animation: %s"), *Anim->GetName());
    }
}

// ======== 按名称查找动画 ========

UWidgetAnimation* UKDDWidget::FindAnimationByName(const FString& AnimName) const
{
    if (AnimName.IsEmpty()) return nullptr;

    // 1) 通过反射查找同名 FObjectProperty（蓝图生成的动画属性）
    FProperty* Prop = GetClass()->FindPropertyByName(*AnimName);
    if (Prop)
    {
        FObjectProperty* ObjProp = CastField<FObjectProperty>(Prop);
        if (ObjProp)
        {
            UWidgetAnimation* Anim = Cast<UWidgetAnimation>(ObjProp->GetObjectPropertyValue_InContainer(this));
            if (Anim) return Anim;
        }
    }

    // 2) 兜底：扫描 WidgetBlueprintGeneratedClass::Animations 按名字匹配
    if (UWidgetBlueprintGeneratedClass* BGClass = Cast<UWidgetBlueprintGeneratedClass>(GetClass()))
    {
        for (UWidgetAnimation* Anim : BGClass->Animations)
        {
            if (Anim && Anim->GetFName() == *AnimName)
            {
                return Anim;
            }
        }
    }

    return nullptr;
}

// ======== 编辑器测试按钮 ========
//
// ★ 原理（参考 RedApp KFUIWidget）：
//   UE5.8 UMG 蓝图编辑器中，CallInEditor 在"设计器预览 Widget 实例"上执行，
//   该实例有有效的 GetWorld()（Editor World）。在此实例上：
//   1. PlayAnimation(Anim) 正常创建 FWidgetAnimationState 加入 ActiveAnimations
//   2. 但设计时没有世界 tick，AnimationState 不会被驱动
//   3. 用 GetWorld()->TimerManager.SetTimer() 设一个0.017s定时器
//   4. 定时器回调中手动调用 EditorAnimState->Tick(dt) + Invalidate 强制重绘
//   5. 动画完成回调停止定时器

#if WITH_EDITOR
#include "Animation/WidgetAnimationState.h"

// ======== 世界定时器管理（RedApp KFUIWidget 方案） ========

void UKDDWidget::Editor_TickPreviewAnim()
{
    if (IsDesignTime() && GetWorld())
    {
        int64 NowTicks = FDateTime::Now().GetTicks();
        float dt = (float)(NowTicks - EditorPrevTicks) / ETimespan::TicksPerSecond;

        if (EditorAnimState.IsValid())
        {
            EditorAnimState->Tick(dt);
        }

        if (TSharedPtr<SWidget> CachedWidget = GetCachedWidget())
        {
            CachedWidget->Invalidate(EInvalidateWidget::Paint);
        }

        EditorPrevTicks = NowTicks;
    }
}

void UKDDWidget::Editor_StartPreviewAnimTimer()
{
    if (IsDesignTime() && GetWorld())
    {
        EditorPrevTicks = FDateTime::Now().GetTicks();
        GetWorld()->GetTimerManager().SetTimer(EditorPreviewTimerHandle, this, &UKDDWidget::Editor_TickPreviewAnim, 0.017f, true);
    }
}

void UKDDWidget::Editor_StopPreviewAnimTimer()
{
    if (IsDesignTime() && GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(EditorPreviewTimerHandle);
    }
}

void UKDDWidget::Editor_OnAnimFinished()
{
    EditorAnimState.Reset();
    Editor_StopPreviewAnimTimer();
}

// ======== CallInEditor 入口 ========

void UKDDWidget::Editor_PlayShowAnim()
{
    UE_LOG(LogTemp, Log, TEXT("[KDDWidget] ▶ Show: this=%s World=%s"), *GetName(),
        GetWorld() ? *GetWorld()->GetName() : TEXT("null"));

    UWidgetAnimation* Anim = GetShowAnimation();
    if (!Anim)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KDDWidget] ShowAnim not found in blueprint"));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("[KDDWidget] ShowAnim found: %s"), *Anim->GetName());

    // 停止之前的预览
    Editor_StopPreviewAnimTimer();
    EditorAnimState.Reset();

    // 绑定完成回调
    UnbindFromAnimationFinished(Anim, EditorAnimFinishEvt);
    EditorAnimFinishEvt.BindDynamic(this, &UKDDWidget::Editor_OnAnimFinished);
    BindToAnimationFinished(Anim, EditorAnimFinishEvt);

    // 播放动画（创建动画状态）
    PlayAnimation(Anim, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f, false);

    // 获取 FWidgetAnimationState 句柄用于手动 tick
    EditorAnimState = GetAnimationState(Anim);
    UE_LOG(LogTemp, Log, TEXT("[KDDWidget] AnimState valid=%d"), EditorAnimState.IsValid());

    // 启动世界定时器驱动动画
    Editor_StartPreviewAnimTimer();
    UE_LOG(LogTemp, Log, TEXT("[KDDWidget] Preview timer started"));
}

void UKDDWidget::Editor_PlayLoopAnim()
{
    UE_LOG(LogTemp, Log, TEXT("[KDDWidget] ▶ Loop: this=%s"), *GetName());

    UWidgetAnimation* Anim = GetLoopAnimation();
    if (!Anim)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KDDWidget] LoopAnim not found"));
        return;
    }

    Editor_StopPreviewAnimTimer();
    EditorAnimState.Reset();

    // 循环动画不需要完成回调
    PlayAnimation(Anim, 0.0f, 0, EUMGSequencePlayMode::Forward, 1.0f, false);
    EditorAnimState = GetAnimationState(Anim);

    Editor_StartPreviewAnimTimer();
}

void UKDDWidget::Editor_PlayHideAnim()
{
    UE_LOG(LogTemp, Log, TEXT("[KDDWidget] ▶ Hide: this=%s"), *GetName());

    UWidgetAnimation* Anim = GetHideAnimation();
    if (!Anim)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KDDWidget] HideAnim not found"));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("[KDDWidget] HideAnim found: %s"), *Anim->GetName());

    Editor_StopPreviewAnimTimer();
    EditorAnimState.Reset();

    UnbindFromAnimationFinished(Anim, EditorAnimFinishEvt);
    EditorAnimFinishEvt.BindDynamic(this, &UKDDWidget::Editor_OnAnimFinished);
    BindToAnimationFinished(Anim, EditorAnimFinishEvt);

    PlayAnimation(Anim, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f, false);
    EditorAnimState = GetAnimationState(Anim);

    Editor_StartPreviewAnimTimer();
}
#endif
