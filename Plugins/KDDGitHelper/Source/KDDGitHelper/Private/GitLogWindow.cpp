#include "GitLogWindow.h"

#include "GenericPlatform/GenericApplication.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "KDDGitLogWindow"

// 居中到主显示器
void CenterSlateWindow(TSharedRef<SWindow> Window)
{
	FDisplayMetrics Metrics;
	FSlateApplication::Get().GetDisplayMetrics(Metrics);
	if (Metrics.PrimaryDisplayWidth > 0 && Metrics.PrimaryDisplayHeight > 0)
	{
		const FVector2D WindowSize = Window->GetClientSizeInScreen();
		const FVector2D Pos(
			(Metrics.PrimaryDisplayWidth - WindowSize.X) * 0.5f,
			(Metrics.PrimaryDisplayHeight - WindowSize.Y) * 0.5f);
		Window->MoveWindowTo(Pos);
	}
}

// 居中弹出统一日志窗口
void ShowKDDGitLogWindow(const TArray<FGitCommandLog>& Logs)
{
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("KDDGitLogs", "KDD Git Logs"))
		.ClientSize(FVector2D(900, 520))
		.SupportsMinimize(false)
		.SupportsMaximize(false);

	Window->SetContent(SNew(SKDDGitLogWindow).Logs(Logs));

	FSlateApplication::Get().AddWindow(Window);
	CenterSlateWindow(Window);
}

void SKDDGitLogWindow::Construct(const FArguments& InArgs)
{
	for (const FGitCommandLog& L : InArgs._Logs)
	{
		Items.Add(MakeShared<FGitCommandLog>(L));
	}

	ChildSlot
	[
		SNew(SVerticalBox)
		// 表头：Time / Command / Message
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(8, 4)
			[ SNew(STextBlock).Text(LOCTEXT("ColTime", "Time")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 10)) ]
			+ SHorizontalBox::Slot().FillWidth(0.30f).Padding(8, 4)
			[ SNew(STextBlock).Text(LOCTEXT("ColCommand", "Command")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 10)) ]
			+ SHorizontalBox::Slot().FillWidth(0.70f).Padding(8, 4)
			[ SNew(STextBlock).Text(LOCTEXT("ColMessage", "Message")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 10)) ]
		]
		// 日志列表
		+ SVerticalBox::Slot().FillHeight(0.6f)
		[
			SAssignNew(ListView, SListView<TSharedPtr<FGitCommandLog>>)
			.ItemHeight(24)
			.ListItemsSource(&Items)
			.OnGenerateRow(this, &SKDDGitLogWindow::HandleGenerateRow)
			.OnSelectionChanged(this, &SKDDGitLogWindow::HandleSelectionChanged)
			.SelectionMode(ESelectionMode::Single)
		]
		// 日志详情标题
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 6, 8, 2)
		[ SNew(STextBlock).Text(LOCTEXT("DetailTitle", "日志详情")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 10)) ]
		// 日志详情框（点击上行消息后显示完整 message）
		+ SVerticalBox::Slot().FillHeight(0.4f).Padding(8, 2, 8, 8)
		[
			SAssignNew(DetailBox, SMultiLineEditableText)
			.IsReadOnly(true)
			.Text(FText::GetEmpty())
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
		]
	];
}

TSharedRef<ITableRow> SKDDGitLogWindow::HandleGenerateRow(TSharedPtr<FGitCommandLog> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	const FSlateColor MsgColor = Item->bSuccess
		? FSlateColor(FColor(0x8BC24AFF))   // 成功绿
		: FSlateColor(FColor(0xE62532FF));  // 失败红

	return SNew(STableRow<TSharedPtr<FGitCommandLog>>, OwnerTable)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(8, 2).VAlign(VAlign_Center)
		[ SNew(STextBlock).Text(FText::FromString(Item->Time.ToString(TEXT("%H:%M:%S")))).Font(FCoreStyle::GetDefaultFontStyle("Regular", 9)) ]
		+ SHorizontalBox::Slot().FillWidth(0.30f).Padding(8, 2).VAlign(VAlign_Center)
		[ SNew(STextBlock).Text(FText::FromString(Item->Command)).Font(FCoreStyle::GetDefaultFontStyle("Regular", 9)).AutoWrapText(true) ]
		+ SHorizontalBox::Slot().FillWidth(0.70f).Padding(8, 2).VAlign(VAlign_Center)
		[ SNew(STextBlock).Text(FText::FromString(Item->Message)).ColorAndOpacity(MsgColor).Font(FCoreStyle::GetDefaultFontStyle("Regular", 9)).AutoWrapText(true) ]
	];
}

void SKDDGitLogWindow::HandleSelectionChanged(TSharedPtr<FGitCommandLog> Item, ESelectInfo::Type)
{
	if (Item.IsValid() && DetailBox.IsValid())
	{
		DetailBox->SetText(FText::FromString(Item->Detail));
	}
}

#undef LOCTEXT_NAMESPACE
