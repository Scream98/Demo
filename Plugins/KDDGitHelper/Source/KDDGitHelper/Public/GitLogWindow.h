#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "GitOperations.h"

// 弹出一个居中的 "KDD Git Logs" 结果窗口（Time / Command / Message + 日志详情）
void ShowKDDGitLogWindow(const TArray<FGitCommandLog>& Logs);

// 把已创建的 SWindow 居中到主显示器
void CenterSlateWindow(TSharedRef<SWindow> Window);

class SKDDGitLogWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SKDDGitLogWindow) {}
		SLATE_ARGUMENT(TArray<FGitCommandLog>, Logs)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<SListView<TSharedPtr<FGitCommandLog>>> ListView;
	TSharedPtr<SMultiLineEditableText> DetailBox;
	TArray<TSharedPtr<FGitCommandLog>> Items;

	TSharedRef<ITableRow> HandleGenerateRow(TSharedPtr<FGitCommandLog> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void HandleSelectionChanged(TSharedPtr<FGitCommandLog> Item, ESelectInfo::Type SelectInfo);
};
