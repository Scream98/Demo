#include "GitHistoryWindow.h"
#include "GitLogWindow.h"

#include "Widgets/SWindow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "KDDGitHistoryWindow"

void SKDDGitHistoryWindow::Construct(const FArguments& InArgs)
{
	Files = InArgs._Files;
	for (const FGitFileHistoryEntry& E : InArgs._History)
	{
		Items.Add(MakeShared<FGitFileHistoryEntry>(E));
	}
	const FString Label = InArgs._FileLabel;

	ChildSlot
	[
		SNew(SVerticalBox)
		// 标题
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 6)
		[ SNew(STextBlock).Text(FText::Format(LOCTEXT("TitleFmt", "文件历史：{0}"), FText::FromString(Label))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11)) ]
		// 表头：Hash / Author / Date / Subject / 操作
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(8, 4) [ SNew(STextBlock).Text(LOCTEXT("HHash", "Hash")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 10)) ]
			+ SHorizontalBox::Slot().FillWidth(0.18f).Padding(8, 4) [ SNew(STextBlock).Text(LOCTEXT("HAuthor", "Author")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 10)) ]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8, 4) [ SNew(STextBlock).Text(LOCTEXT("HDate", "Date")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 10)) ]
			+ SHorizontalBox::Slot().FillWidth(0.5f).Padding(8, 4) [ SNew(STextBlock).Text(LOCTEXT("HSubject", "Subject")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 10)) ]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8, 4) [ SNew(STextBlock).Text(LOCTEXT("HAction", "操作")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 10)) ]
		]
		// 历史列表
		+ SVerticalBox::Slot().FillHeight(1.0f)
		[
			SNew(SListView<TSharedPtr<FGitFileHistoryEntry>>)
			.ItemHeight(28)
			.ListItemsSource(&Items)
			.OnGenerateRow(this, &SKDDGitHistoryWindow::HandleGenerateRow)
			.SelectionMode(ESelectionMode::Single)
		]
	];
}

TSharedRef<ITableRow> SKDDGitHistoryWindow::HandleGenerateRow(TSharedPtr<FGitFileHistoryEntry> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedPtr<FGitFileHistoryEntry> Pinned = Item;
	return SNew(STableRow<TSharedPtr<FGitFileHistoryEntry>>, OwnerTable)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(8, 2).VAlign(VAlign_Center)
		[ SNew(STextBlock).Text(FText::FromString(Item->ShortHash)).Font(FCoreStyle::GetDefaultFontStyle("Regular", 9)) ]
		+ SHorizontalBox::Slot().FillWidth(0.18f).Padding(8, 2).VAlign(VAlign_Center)
		[ SNew(STextBlock).Text(FText::FromString(Item->Author)).Font(FCoreStyle::GetDefaultFontStyle("Regular", 9)).AutoWrapText(true) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(8, 2).VAlign(VAlign_Center)
		[ SNew(STextBlock).Text(FText::FromString(Item->Date)).Font(FCoreStyle::GetDefaultFontStyle("Regular", 9)) ]
		+ SHorizontalBox::Slot().FillWidth(0.5f).Padding(8, 2).VAlign(VAlign_Center)
		[ SNew(STextBlock).Text(FText::FromString(Item->Subject)).Font(FCoreStyle::GetDefaultFontStyle("Regular", 9)).AutoWrapText(true) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(8, 2).VAlign(VAlign_Center)
		[ SNew(SButton).Text(LOCTEXT("Pull", "拉取到本地")).OnClicked(this, &SKDDGitHistoryWindow::HandlePullVersion, Pinned) ]
	];
}

FReply SKDDGitHistoryWindow::HandlePullVersion(TSharedPtr<FGitFileHistoryEntry> Item)
{
	if (!Item.IsValid())
	{
		return FReply::Handled();
	}
	// 把该历史版本检出到工作区，刷新资产，并弹出统一日志窗口
	TArray<FGitCommandLog> Logs = FGitOperations::CheckoutVersionLogged(Files, Item->Hash);
	FGitOperations::RescanFiles(Files);
	ShowKDDGitLogWindow(Logs);
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
