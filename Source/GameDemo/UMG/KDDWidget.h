#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnLuaInterface.h"
#include "Animation/WidgetAnimation.h"
struct FWidgetAnimationState;
#include "KDDWidget.generated.h"

/**
 * KDD 控件基类（继承自 UUserWidget + IUnLuaInterface）。
 * 设计为可复用组件的基类（按钮、列表项、面板等），
 * 拥有生命周期 + 动画系统。
 *
 * 实现了 IUnLuaInterface，配合 KDDModuleLocator，
 * 在 Widget 被创建时 UnLua 自动从配表中查找并绑定对应的 Lua 模块。
 *
 * ★ 生命周期（由子类覆盖）：
 *   NativeConstruct → OnCreate → OnOpen
 *   NativeDestruct  → OnClose → OnDestroy
 *
 * ★ 动画功能：
 *   1. 自动识别蓝图中名为 ShowAnim / HideAnim / LoopAnim 的动画
 *   2. NativeConstruct 时自动播放 ShowAnim，NativeDestruct 时自动播放 HideAnim
 *   3. 支持自定义 Open / Close / Loop 三种动画（可配置动画名）
 *   4. 编辑器模式下提供 3 个测试按钮
 */
UCLASS(Blueprintable, BlueprintType, Abstract, meta=(Category="KDD"))
class UKDDWidget : public UUserWidget, public IUnLuaInterface
{
    GENERATED_BODY()

public:
    UKDDWidget(const FObjectInitializer& ObjectInitializer);

    // IUnLuaInterface: 标记此类需要 UnLua 自动绑定，ModuleLocator 会从配表查找路径
    virtual FString GetModuleName_Implementation() const override;

    // ======== 动画开关 ========

    /** 是否启用动画（默认 true） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="KDD|Animation")
    bool bEnableAnimation;

    // ======== 自定义动画名配置（文本输入框） ========

    /** 自定义打开动画名（为空时使用默认 ShowAnim） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="KDD|Animation", meta=(DisplayName="自定义打开动画（默认ShowAnim）"))
    FString CustomShowAnimName;

    /** 自定义关闭动画名（为空时使用默认 HideAnim） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="KDD|Animation", meta=(DisplayName="自定义关闭动画（默认HideAnim）"))
    FString CustomHideAnimName;

    /** 自定义循环动画名（为空时使用默认 LoopAnim） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="KDD|Animation", meta=(DisplayName="自定义循环动画（默认LoopAnim）"))
    FString CustomLoopAnimName;

    // ======== 动画播放接口 ========

    /** 播放 Show 动画（优先自动识别 ShowAnim，回退到 CustomShowAnimName） */
    UFUNCTION(BlueprintCallable, Category="KDD|Animation")
    void PlayShowAnimation();

    /** 播放 Hide 动画（优先自动识别 HideAnim，回退到 CustomHideAnimName） */
    UFUNCTION(BlueprintCallable, Category="KDD|Animation")
    void PlayHideAnimation();

    /** 播放 Loop 动画（使用 CustomLoopAnimName，0 次 = 无限循环） */
    UFUNCTION(BlueprintCallable, Category="KDD|Animation")
    void PlayLoopAnimation();

    /** 停止 Loop 动画 */
    UFUNCTION(BlueprintCallable, Category="KDD|Animation")
    void StopLoopAnimation();

    /** 获取显示动画（由子类覆盖返回动画实例） */
    UFUNCTION(BlueprintNativeEvent, Category="KDD|Animation")
    UWidgetAnimation* GetShowAnimation() const;
    virtual UWidgetAnimation* GetShowAnimation_Implementation() const;

    /** 获取隐藏动画 */
    UFUNCTION(BlueprintNativeEvent, Category="KDD|Animation")
    UWidgetAnimation* GetHideAnimation() const;
    virtual UWidgetAnimation* GetHideAnimation_Implementation() const;

    /** 获取循环动画（自动识别 LoopAnim → 自定义名 → 兜底 LoopAnim） */
    UWidgetAnimation* GetLoopAnimation() const;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    /** 子类可覆盖的详细生命周期 */
    virtual void OnCreate();
    virtual void OnOpen();
    virtual void OnClose();
    virtual void OnRefresh();
    virtual void OnDestroy();

    /** 获取指定名称的动画 */
    UWidgetAnimation* FindAnimationByName(const FString& AnimName) const;

private:
    /** 自动识别的动画缓存 */
    UPROPERTY(Transient)
    UWidgetAnimation* AutoShowAnim;
    UPROPERTY(Transient)
    UWidgetAnimation* AutoHideAnim;
    UPROPERTY(Transient)
    UWidgetAnimation* AutoLoopAnim;

    /** 查找并缓存自动识别的动画 */
    void CacheAutoAnimations();

#if WITH_EDITOR
    // ======== 编辑器测试按钮（仅在 Details 面板中显示） ========

    UFUNCTION(CallInEditor, Category="KDD|Animation|Test", meta=(DisplayName="▶ 播放 Show 动画"))
    void Editor_PlayShowAnim();

    UFUNCTION(CallInEditor, Category="KDD|Animation|Test", meta=(DisplayName="▶ 播放 Loop 动画"))
    void Editor_PlayLoopAnim();

    UFUNCTION(CallInEditor, Category="KDD|Animation|Test", meta=(DisplayName="▶ 播放 Hide 动画"))
    void Editor_PlayHideAnim();

    /** 编辑器动画预览：世界定时器手动 tick FWidgetAnimationState */
    void Editor_TickPreviewAnim();
    void Editor_StartPreviewAnimTimer();
    void Editor_StopPreviewAnimTimer();

    /** 动画播放完成回调 */
    UFUNCTION()
    void Editor_OnAnimFinished();

    // ======== 实例状态 ========
    FTimerHandle EditorPreviewTimerHandle;
    int64 EditorPrevTicks;
    FWidgetAnimationDynamicEvent EditorAnimFinishEvt;
    TSharedPtr<FWidgetAnimationState> EditorAnimState;
#endif
};
