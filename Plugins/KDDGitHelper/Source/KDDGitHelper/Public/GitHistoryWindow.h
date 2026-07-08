#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "GitOperations.h"

// 文件历史弹窗：列出提交历史，每行可"拉取到本地"
class SKDDGitHistoryWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SKDDGitHistoryWindow) {}
		SLATE_ARGUMENT(TArray<FGitFileHistoryEntry>, History)
		SLATE_ARGUMENT(TArray<FString>, Files)
		SLATE_ARGUMENT(FString, FileLabel)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TArray<FString> Files;
	TArray<TSharedPtr<FGitFileHistoryEntry>> Items;

	TSharedRef<ITableRow> HandleGenerateRow(TSharedPtr<FGitFileHistoryEntry> Item, const TSharedRef<STableViewBase>& OwnerTable);
	FReply HandlePullVersion(TSharedPtr<FGitFileHistoryEntry> Item);
};
