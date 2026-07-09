#pragma once

#include "CoreMinimal.h"
#include "UMG/KDDView.h"
#include "Animation/WidgetAnimation.h"
#include "KDDWidget.generated.h"

/**
 * KDD 控件基类（继承自 UKDDView）。
 * 设计为可复用组件的基类（按钮、列表项、面板等），
 * 拥有 UKDDView 的所有能力 + 动画系统。
 *
 * ★ 动画功能：
 *   1. 自动识别蓝图中名为 ShowAnim / HideAnim 的动画
 *   2. NativeConstruct 时自动播放 ShowAnim，NativeDestruct 时自动播放 HideAnim
 *   3. 支持自定义 Open / Close / Loop 三种动画（可配置动画名）
 *   4. 编辑器模式下提供 3 个测试按钮
 */
UCLASS(Blueprintable, BlueprintType, Abstract, meta=(Category="KDD"))
class UKDDWidget : public UKDDView
{
    GENERATED_BODY()

public:
    UKDDWidget(const FObjectInitializer& ObjectInitializer);

    // IUnLuaInterface
    virtual FString GetModuleName_Implementation() const override;

    // ======== 动画开关 ========

    /** 是否启用动画（默认 true） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="KDD|Animation")
    bool bEnableAnimation;

    // ======== 自定义动画名配置（文本输入框） ========

    /** 打开动画名 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="KDD|Animation", meta=(DisplayName="Open Anim Name"))
    FString CustomShowAnimName;

    /** 关闭动画名 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="KDD|Animation", meta=(DisplayName="Close Anim Name"))
    FString CustomHideAnimName;

    /** 循环动画名 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="KDD|Animation", meta=(DisplayName="Loop Anim Name"))
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

    // ======== 覆盖 GetShowAnimation / GetHideAnimation ========

    virtual UWidgetAnimation* GetShowAnimation_Implementation() const override;
    virtual UWidgetAnimation* GetHideAnimation_Implementation() const override;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    /** 覆盖 OnOpen：自动播放 Show 动画 */
    virtual void OnOpen() override;

    /** 覆盖 OnClose：自动播放 Hide 动画 */
    virtual void OnClose() override;

private:
    /** 自动识别的动画缓存 */
    UWidgetAnimation* AutoShowAnim;
    UWidgetAnimation* AutoHideAnim;

    /** 查找并缓存自动识别的动画 */
    void CacheAutoAnimations();

#if WITH_EDITOR
    // ======== 编辑器测试按钮（仅在 Details 面板中显示） ========

    UFUNCTION(CallInEditor, Category="KDD|Animation|Test", meta=(DisplayName="▶ Play Show Animation"))
    void Editor_PlayShowAnim();

    UFUNCTION(CallInEditor, Category="KDD|Animation|Test", meta=(DisplayName="▶ Play Loop Animation"))
    void Editor_PlayLoopAnim();

    UFUNCTION(CallInEditor, Category="KDD|Animation|Test", meta=(DisplayName="▶ Play Hide Animation"))
    void Editor_PlayHideAnim();
#endif
};
