#include "KDDGitHelperModule.h"
#include "GitOperations.h"
#include "GitLogWindow.h"
#include "GitHistoryWindow.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/Commands/UIAction.h"
#include "Misc/MessageDialog.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"
#include "HAL/IConsoleManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include "Editor.h"
#include "UObject/SavePackage.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"
#include "Serialization/Archive.h"
#include "UObject/PackageReload.h"
#include "LevelEditor.h"
#include "Engine/World.h"
#include "FileHelpers.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogKDDGitHelperUI, Log, All);

#define LOCTEXT_NAMESPACE "KDDGitHelper"

void FKDDGitHelperModule::StartupModule()
{
	// 开启 file.allowdeleteopenfiles
	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("file.allowdeleteopenfiles")))
	{
		CVar->Set(1);
		UE_LOG(LogKDDGitHelperUI, Log, TEXT("KDDGitHelper: 已开启 file.allowdeleteopenfiles"));
	}
	else
	{
		UE_LOG(LogKDDGitHelperUI, Warning, TEXT("KDDGitHelper: 未找到 CVar file.allowdeleteopenfiles"));
	}

	RegisterContentBrowserContextMenu();
}

void FKDDGitHelperModule::ShutdownModule()
{
}

void FKDDGitHelperModule::RegisterContentBrowserContextMenu()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	ContentBrowserModule.GetAllAssetViewContextMenuExtenders().Add(
		FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FKDDGitHelperModule::OnExtendContentBrowserAssetSelectionMenu)
	);
	UE_LOG(LogKDDGitHelperUI, Log, TEXT("KDDGitHelper: 已注册 Content Browser 右键菜单扩展"));
}

TSharedRef<FExtender> FKDDGitHelperModule::OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets)
{
	const TSharedRef<FExtender> Extender = MakeShared<FExtender>();
	const TArray<FAssetData> Assets = SelectedAssets;
	const TSharedPtr<FUICommandList> CommandList = MakeShared<FUICommandList>();

	Extender->AddMenuExtension(
		"GetAssetActions",
		EExtensionHook::After,
		CommandList,
		FMenuExtensionDelegate::CreateLambda([this, Assets](FMenuBuilder& MenuBuilder)
		{
			MenuBuilder.AddSubMenu(
				NSLOCTEXT("KDDGitHelper", "KDDGit", "KDDGit"),
				NSLOCTEXT("KDDGitHelper", "KDDGitTooltip", "Git 操作"),
				FNewMenuDelegate::CreateLambda([this, Assets](FMenuBuilder& SubMenu)
				{
					// Git 更新
					SubMenu.AddMenuEntry(
						NSLOCTEXT("KDDGitHelper", "Update", "Git更新"),
						NSLOCTEXT("KDDGitHelper", "UpdateTooltip", "从远程更新选中资产"),
						FSlateIcon(),
						FUIAction(FExecuteAction::CreateRaw(this, &FKDDGitHelperModule::ExecuteUpdate, Assets))
					);
					// Git 历史
					SubMenu.AddMenuEntry(
						NSLOCTEXT("KDDGitHelper", "History", "Git历史"),
						NSLOCTEXT("KDDGitHelper", "HistoryTooltip", "查看选中资产的提交历史"),
						FSlateIcon(),
						FUIAction(FExecuteAction::CreateRaw(this, &FKDDGitHelperModule::ExecuteHistory, Assets))
					);
				}),
				false,
				FSlateIcon()
			);
		})
	);

	return Extender;
}

void FKDDGitHelperModule::ExecuteUpdate(TArray<FAssetData> SelectedAssets)
{
	const TArray<FString> Files = FGitOperations::ResolveAssetFilePaths(SelectedAssets);
	if (Files.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("KDDGitHelper", "NoFiles", "未解析到可选中的资产文件。"));
		return;
	}

	const TArray<FGitCommandLog> Logs = FGitOperations::UpdateLogged(Files);
	FGitOperations::RescanFiles(Files);

	ReloadRevertedAssets(SelectedAssets);

	ShowKDDGitLogWindow(Logs);
}

void FKDDGitHelperModule::ExecuteHistory(TArray<FAssetData> SelectedAssets)
{
	const TArray<FString> Files = FGitOperations::ResolveAssetFilePaths(SelectedAssets);
	if (Files.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("KDDGitHelper", "NoFiles", "未解析到可选中的资产文件。"));
		return;
	}

	TArray<FGitFileHistoryEntry> History;
	if (!FGitOperations::GetFileHistory(Files, History))
	{
		FGitCommandLog Err;
		Err.Time = FDateTime::Now();
		Err.Command = TEXT("git log");
		Err.bSuccess = false;
		Err.Detail = TEXT("获取文件历史失败：可能该文件不在 Git 仓库内，或 git 命令执行出错。请确认已在 UGit 登录且仓库路径正确。");
		Err.Message = TEXT("获取历史失败");
		TArray<FGitCommandLog> ErrLogs;
		ErrLogs.Add(Err);
		ShowKDDGitLogWindow(ErrLogs);
		return;
	}

	if (History.Num() == 0)
	{
		FGitCommandLog Info;
		Info.Time = FDateTime::Now();
		Info.Command = TEXT("git log");
		Info.bSuccess = true;
		Info.Detail = TEXT("该文件没有提交历史。");
		Info.Message = TEXT("无历史记录");
		TArray<FGitCommandLog> InfoLogs;
		InfoLogs.Add(Info);
		ShowKDDGitLogWindow(InfoLogs);
		return;
	}

	const FString FileLabel = FPaths::GetCleanFilename(Files[0]) + (Files.Num() > 1 ? FString::Printf(TEXT(" (等 %d 个文件)"), Files.Num()) : FString());

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("KDDGitHistoryTitle", "KDD Git 历史"))
		.ClientSize(FVector2D(900, 520))
		.SupportsMinimize(false)
		.SupportsMaximize(false);
	Window->SetContent(SNew(SKDDGitHistoryWindow).History(History).Files(Files).FileLabel(FileLabel));
	FSlateApplication::Get().AddWindow(Window);
	CenterSlateWindow(Window);
}

void FKDDGitHelperModule::ReloadRevertedAssets(const TArray<FAssetData>& SelectedAssets)
{
	if (!GEditor)
	{
		return;
	}
	
	TArray<FReloadPackageData> PackagesToReload;
	for (const FAssetData& Asset : SelectedAssets)
	{
		const FString PackageName = Asset.PackageName.ToString();
		if (PackageName.IsEmpty())
		{
			continue;
		}

		// 对地图/关卡：关闭再重新打开，确保编辑器完全刷新
		if (Asset.GetClass() && Asset.GetClass()->IsChildOf(UWorld::StaticClass()))
		{
			UWorld* CurrentWorld = nullptr;
			for (const FWorldContext& Context : GEditor->GetWorldContexts())
			{
				if (Context.WorldType == EWorldType::Editor)
				{
					CurrentWorld = Context.World();
					break;
				}
			}

			if (CurrentWorld && CurrentWorld->GetPackage()->GetName() == PackageName)
			{
				UE_LOG(LogKDDGitHelperUI, Log, TEXT("KDDGitHelper: Map %s is currently open, scheduling reload..."), *PackageName);
				// 当前打开的就是这个地图 → 用 EditorLoadingAndSavingUtils 重新加载
				// 延迟 0.5s 执行，等 git checkout 完全落盘
				FTimerHandle Handle;
				CurrentWorld->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([PackageName]()
				{
					UEditorLoadingAndSavingUtils::LoadMap(PackageName);
				}), 0.5f, false);
				continue;
			}
		}

		UPackage* Package = FindPackage(nullptr, *PackageName);
		if (Package)
		{
			PackagesToReload.Add(FReloadPackageData(Package, LOAD_None));
			UE_LOG(LogKDDGitHelperUI, Log, TEXT("KDDGitHelper: Found loaded package %s"), *PackageName);
		}
	}
	
	if (PackagesToReload.Num() > 0)
	{
		UE_LOG(LogKDDGitHelperUI, Log, TEXT("KDDGitHelper: Reloading %d package(s)..."), PackagesToReload.Num());
		TArray<UPackage*> ReloadedPackages;
		ReloadPackages(PackagesToReload, ReloadedPackages);
	}

	TArray<FString> FilePaths = FGitOperations::ResolveAssetFilePaths(SelectedAssets);
	FGitOperations::RescanFiles(FilePaths);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FKDDGitHelperModule, KDDGitHelper)
