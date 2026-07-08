#include "KDDWidget.h"

UKDDWidget::UKDDWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

FString UKDDWidget::GetModuleName_Implementation() const
{
    return TEXT("");
}

void UKDDWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UKDDWidget::NativeDestruct()
{
    Super::NativeDestruct();
}

void UKDDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}
