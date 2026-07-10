#include "UIPacker.h"
#include "UIPackerStyle.h"
#include "UIPackerCommands.h"
#include "Misc/MessageDialog.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformFileManager.h"
#include "LevelEditor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "AssetToolsModule.h"
#include "WidgetBlueprint.h"
#include "UMG.h"
#include "Commandlets/Commandlet.h"
#include "IPythonScriptPlugin.h"
#include "Settings/EditorLoadingSavingSettings.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "EditorReimportHandler.h"
#include "FileHelpers.h"
#include "ObjectTools.h"
#include "PackageTools.h"
#include "PaperSprite.h"
#include "Interfaces/IPluginManager.h"
#include "WidgetBlueprintEditor.h"
#include "Misc/FeedbackContext.h"
#include "ToolMenus.h"

DEFINE_LOG_CATEGORY(KDUIPackerLog);

static const FName UIPackerTabName("UIPacker");

#define LOCTEXT_NAMESPACE "FUIPackerModule"

void FUIPackerModule::StartupModule()
{
	FUIPackerStyle::Initialize();
	FUIPackerStyle::ReloadTextures();

	FUIPackerCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FUIPackerCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FUIPackerModule::PluginButtonClicked),
		FCanExecuteAction());

	// 1. 窗口菜单顶置（比"关卡编辑器"还高）
	// 2. 主工具栏（Play 按钮那一栏）
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (ToolMenus)
	{
		// 窗口菜单顶置
		UToolMenu* WindowMenu = ToolMenus->ExtendMenu("LevelEditor.MainMenu.Window");
		if (WindowMenu)
		{
			FToolMenuSection& Section = WindowMenu->AddSection("UIPacker", FText::GetEmpty(),
				FToolMenuInsert(NAME_None, EToolMenuInsertType::First));
			Section.AddMenuEntryWithCommandList(FUIPackerCommands::Get().PluginAction, PluginCommands);
		}

		// 主工具栏
		UToolMenu* Toolbar = ToolMenus->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
		if (Toolbar)
		{
			FToolMenuSection& Section = Toolbar->AddSection("UIPacker");
			FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(
				FUIPackerCommands::Get().PluginAction,
				LOCTEXT("UIPacker_Label", "UIPacker"),
				LOCTEXT("UIPacker_ToolTip", "选中 DesignResource 文件夹后点击，自动打图集"),
				FSlateIcon(FUIPackerStyle::GetStyleSetName(), "UIPacker.PluginAction")
			));
			Entry.SetCommandList(PluginCommands);
		}
	}
}

void FUIPackerModule::ShutdownModule()
{
	FUIPackerStyle::Shutdown();

	FUIPackerCommands::Unregister();
}

void FUIPackerModule::ShutdownWindows()
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();
	const FString AssetType = "PaperSpriteSheet";
	for (int32 AssetIdx = 0; AssetIdx < EditedAssets.Num(); AssetIdx++)
	{
		UObject* EditedAsset = EditedAssets[AssetIdx];
		FString AssetEditClassName = EditedAsset->GetClass()->GetName();
		if (EditedAsset->IsA(UTexture2D::StaticClass())
            || EditedAsset->IsA(UPaperSprite::StaticClass())
            || EditedAsset->IsA(UWidgetBlueprint::StaticClass())
            || AssetEditClassName.Equals(AssetType))
		{
			IAssetEditorInstance* Editor = AssetEditorSubsystem->FindEditorForAsset(EditedAsset, false);
			if (Editor)
			{
				Editor->CloseWindow(EAssetEditorCloseReason::AssetEditorHostClosed);
			}
		}
	}
}

FString FUIPackerModule::GetScriptPath()
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("UIPacker");
	if (Plugin.IsValid())
	{
		return Plugin->GetBaseDir() / TEXT("Content/Python/AutoPackUI.py");
	}
	return FString();
}

/**
 * 取得 SpriteSheet 输出目录在 Content 下的相对路径（如 "UI/SpriteSheet/MyIcons"）
 */
FString FUIPackerModule::GetSavedPath()
{
	return OutputSpriteSheetRelPath;
}

/**
 * 自动导入生成好的图集资源
 */
void FUIPackerModule::ImportAssets()
{
	const FString& InPath = GetSavedPath();
	
	if (ensure(!InPath.IsEmpty()))
	{
		const FString FullPath = TEXT("/Game/") + InPath;
		const FString FilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / InPath / TEXT("*"));

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		
		TArray<FString> FileNames;
		TArray<FString> FileNamesToClean;
		IFileManager::Get().FindFiles(FileNames, *FilePath, true, false);

		SourceData = NewObject<UAutomatedAssetImportData>();
		SourceData->DestinationPath = FullPath;
		SourceData->bReplaceExisting = true;
		// 不指定工厂，让 UE 按 .paper2dsprites 扩展名自动匹配 PaperSpriteSheetImportFactory
		SourceData->Filenames.Empty();
		for (const FString& Path : FileNames)
		{
			FString FullFilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / InPath / FPaths::GetCleanFilename(Path));
			// atlasTex.png 不导入（由 paper2dsprites 导入时自动处理），直接标记清理
			if (Path.EndsWith(TEXT("atlasTex.png"), ESearchCase::IgnoreCase))
			{
				FileNamesToClean.Add(FullFilePath);
				continue;
			}
			// 只以 paper2dsprites 作为导入入口
			if (Path.EndsWith(TEXT(".paper2dsprites"), ESearchCase::IgnoreCase))
			{
				SourceData->Filenames.Add(FullFilePath);
			}
			// 无论导入成功与否，源文件（paper2dsprites、png）都标记清理
			FileNamesToClean.Add(FullFilePath);
		}

		bool bAutoImport = false;
		if (SourceData->Filenames.Num() > 0)
		{
			FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
			TArray<UObject*> ImportedObjects = AssetToolsModule.Get().ImportAssetsAutomated(SourceData);
			if (ImportedObjects.Num() > 0)
			{
				bAutoImport = true;
				UE_LOG(KDUIPackerLog, Log, TEXT("图集导入成功，导入 %d 个资源"), ImportedObjects.Num());
			}
			else
			{
				UE_LOG(KDUIPackerLog, Error, TEXT("图集导入失败，ImportAssetsAutomated 返回 0 个对象"));
			}
		}

		// 清理已消费的源文件（atlasTex.png、atlas.paper2dsprites），只保留 uasset
		for (const FString& CleanPath : FileNamesToClean)
		{
			if (!CleanPath.IsEmpty())
			{
				IFileManager::Get().Delete(*CleanPath);
			}
		}

		if (bAutoImport)
		{
			UE_LOG(KDUIPackerLog, Log, TEXT("图集已自动导入 %s"), *FullPath);
		}
		else
		{
			UE_LOG(KDUIPackerLog, Warning, TEXT("自动导入失败 %s，请手动导入图集"), *FullPath);
		}
	}
}

void FUIPackerModule::GetPackageNamesWithPaths(TArray<FString>& OutPackageNames, const FString& Path)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	new (Filter.PackagePaths) FName(*Path);

	TArray<FAssetData> AssetList;
	AssetRegistryModule.Get().GetAssets(Filter, AssetList);

	TSet<FName> UniquePackageNames;
	for (const FAssetData& AssetData : AssetList)
	{
		FString AssetExtension = FPaths::GetExtension(AssetData.AssetName.ToString());
		if (AssetExtension.Equals(TEXT("paper2dsprites"), ESearchCase::IgnoreCase) ||
			AssetExtension.Equals(TEXT("png"), ESearchCase::IgnoreCase))
		{
			continue;
		}
		UniquePackageNames.Add(AssetData.PackageName);
	}

	for (auto PackageIt = UniquePackageNames.CreateConstIterator(); PackageIt; ++PackageIt)
	{
		OutPackageNames.Add((*PackageIt).ToString());
	}
}

bool FUIPackerModule::CheckIsBpAsset(FName PackageName)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> AssetDatas;
	AssetRegistryModule.Get().GetAssetsByPackageName(PackageName, AssetDatas);
	for (const FAssetData& Item : AssetDatas)
	{
		if (Item.GetClass()->IsChildOf(UWidgetBlueprint::StaticClass()))
		{
			return true;
		}
	}
	return false;
}

void FUIPackerModule::UnLoadAndSavePackage(const FString& CS)
{
	TArray<FString> PackageNames;
	GetPackageNamesWithPaths(PackageNames, CS);

	TArray<UPackage*> Packages;
	TArray<UPackage*> NeedSavePackages;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	UE::AssetRegistry::EDependencyCategory DependencyCategory = UE::AssetRegistry::EDependencyCategory::Package | UE::AssetRegistry::EDependencyCategory::SearchableName;
	for (const FString& FileName : PackageNames)
	{
		UPackage* Package = FindPackage(nullptr, *FileName);
		if (Package)
		{
			Packages.AddUnique(Package);
			if (Package->IsDirty())
			{
				NeedSavePackages.AddUnique(Package);
			}
			TArray<FName> ReferenceNames;
			AssetRegistryModule.Get().GetReferencers(*Package->GetName(), ReferenceNames, DependencyCategory);
			for (const FName& ReferenceName : ReferenceNames)
			{
				UPackage* PackageReference = FindPackage(nullptr, *ReferenceName.ToString());
				if (PackageReference && PackageReference->IsDirty())
				{
					NeedSavePackages.AddUnique(PackageReference);
				}
			}
		}
	}

	if (NeedSavePackages.Num() > 0)
	{
		FEditorFileUtils::PromptForCheckoutAndSave(NeedSavePackages, true, true, nullptr, true);
	}

	if (Packages.Num() > 0)
	{
		UPackageTools::UnloadPackages(Packages);
	}
}

void FUIPackerModule::SetTexQuality(ETextureCompressionQuality quality)
{
	UE_LOG(KDUIPackerLog, Log, TEXT("SetTexQuality"));
	FString ObjPath = TEXT("/Game") / GetSavedPath() / TEXT("Textures/atlasTex.atlasTex");
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	FAssetData Asset = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(*ObjPath));
	if (Asset.IsValid())
	{
		UObject* Obj = Asset.GetAsset();
		if (Cast<UTexture2D>(Obj))
		{
			UTexture2D* Tex = Cast<UTexture2D>(Obj);
			Tex->CompressionQuality = TEnumAsByte<ETextureCompressionQuality>(quality);
			Obj->MarkPackageDirty();
			UPackageTools::SavePackagesForObjects(TArray<UObject*>({ Obj }));
		}
	}
	else
	{
		UE_LOG(KDUIPackerLog, Warning, TEXT("asset invalid %s"), *ObjPath);
	}
}

void FUIPackerModule::SaveAllFile(const FString& CS, bool bReloadDirtyAsset)
{
	TArray<FString> PackageNames;
	GetPackageNamesWithPaths(PackageNames, CS);

	TArray<UPackage*> Packages;
	TArray<UPackage*> NeedReloadages;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	UE::AssetRegistry::EDependencyCategory DependencyCategory = UE::AssetRegistry::EDependencyCategory::Package | UE::AssetRegistry::EDependencyCategory::SearchableName;
	for (const FString& FileName : PackageNames)
	{
		UPackage* Package = FindPackage(nullptr, *FileName);
		if (Package)
		{
			Packages.AddUnique(Package);
			if (bReloadDirtyAsset)
			{
				TArray<FName> ReferenceNames;
				AssetRegistryModule.Get().GetReferencers(*Package->GetName(), ReferenceNames, DependencyCategory);
				for (const FName& ReferenceName : ReferenceNames)
				{
					UPackage* PackageReference = FindPackage(nullptr, *ReferenceName.ToString());
					if (PackageReference && PackageReference->IsDirty())
					{
						UE_LOG(KDUIPackerLog, Log, TEXT("Packagename SaveAllFile tmp:%s"), *ReferenceName.ToString());
						NeedReloadages.AddUnique(PackageReference);
					}
				}
			}
		}
	}

	if (Packages.Num())
	{
		TArray<UPackage*> OutFailedPackages;
		const FEditorFileUtils::EPromptReturnCode Return = FEditorFileUtils::PromptForCheckoutAndSave(Packages, false, false, &OutFailedPackages);

		if (Return != FEditorFileUtils::EPromptReturnCode::PR_Success)
		{
			UE_LOG(KDUIPackerLog, Warning, TEXT("已存在图集未能保存成功"));
		}
	}

	if (NeedReloadages.Num() > 0)
	{
		UPackageTools::ReloadPackages(NeedReloadages);
	}
}

void FUIPackerModule::ReloadAssets()
{
	const FString TargetName = TEXT("/Game/") + GetSavedPath();
	TargetPackageNames.Empty();
	GetPackageNamesWithPaths(TargetPackageNames, TargetName);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<UPackage*> PackagesToReload;
	UE::AssetRegistry::EDependencyCategory DependencyCategory = UE::AssetRegistry::EDependencyCategory::Package;
	for (const FString& FileName : TargetPackageNames)
	{
		UPackage* Package = FindPackage(nullptr, *FileName);
		if (!Package)
		{
			Package = LoadPackage(nullptr, *FileName, LOAD_None);
		}
		if (Package)
		{
			FString Name = Package->GetName();
			TArray<FName> ReferenceNames;
			AssetRegistryModule.Get().GetReferencers(*Name, ReferenceNames, DependencyCategory);
			for (const FName& ReferenceName : ReferenceNames)
			{
				UPackage* PackageReference = FindPackage(nullptr, *ReferenceName.ToString());
				if (PackageReference)
				{
					if (CheckIsBpAsset(PackageReference->GetFName()) || State != EPackerProgress::PreReload)
					{
						PackagesToReload.AddUnique(PackageReference);
					}
				}
			}
		}
	}

	if (PackagesToReload.Num() > 0)
	{
		UPackageTools::ReloadPackages(PackagesToReload);
	}
	TargetPackageNames.Empty();
}

void FUIPackerModule::DeleteNoUsedAssets()
{
	const FString TargetName = TEXT("/Game/") + GetSavedPath();
	TargetPackageNames.Empty();
	GetPackageNamesWithPaths(TargetPackageNames, TargetName);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<UPackage*> PackagesToUnload;
	for (const FString& FileName : TargetPackageNames)
	{
		UPackage* Package = FindPackage(nullptr, *FileName);
		if (Package)
		{
			PackagesToUnload.AddUnique(Package);

			TArray<FName> ReferenceNames;
			AssetRegistryModule.Get().GetReferencers(*Package->GetName(), ReferenceNames, UE::AssetRegistry::EDependencyCategory::Package);
			for (const FName& ReferenceName : ReferenceNames)
			{
				UPackage* PackageReference = FindPackage(nullptr, *ReferenceName.ToString());
				if (PackageReference)
				{
					PackagesToUnload.AddUnique(PackageReference);
				}
			}
		}
	}

	if (PackagesToUnload.Num() > 0)
	{
		UPackageTools::UnloadPackages(PackagesToUnload);
	}

	// 只清理 Frames 和 Textures 子目录下的旧资产，保留根目录
	auto DeleteAssetsInSubPath = [&](const FString& SubPath)
	{
		TArray<FAssetData> AssetList;
		AssetRegistryModule.Get().GetAssetsByPath(FName(*SubPath), AssetList, true);
		for (const FAssetData& AssetData : AssetList)
		{
			UObject* Object = AssetData.GetAsset();
			if (AssetData.IsValid() && Object != nullptr)
			{
				AssetRegistryModule.AssetDeleted(Object);
				ObjectTools::DeleteSingleObject(Object, false);
				Object->MarkAsGarbage();
			}
		}
	};

	DeleteAssetsInSubPath(TargetName + TEXT("/Frames"));
	DeleteAssetsInSubPath(TargetName + TEXT("/Textures"));

	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
}


void FUIPackerModule::PluginButtonClicked()
{
	State = EPackerProgress::None;
#if PLATFORM_WINDOWS

	if (!IPythonScriptPlugin::Get()->IsPythonAvailable())
	{
		UE_LOG(KDUIPackerLog, Error, TEXT("** Python 命令不可用 **"));
		return;
	}

	// 检查 TexturePacker.exe 是否存在
	FString ScriptPath = GetScriptPath();
	FString ToolPath = FPaths::GetPath(ScriptPath) / TEXT("../TexturePacker/bin/TexturePacker.exe");
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*ToolPath))
	{
		UE_LOG(KDUIPackerLog, Error, TEXT("** 找不到 TexturePacker.exe，请检查路径: %s **"), *ToolPath);
		return;
	}

	TArray<FString> selectedFolders;
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowserModule.Get().GetSelectedFolders(selectedFolders);
	if (selectedFolders.Num() == 0)
		ContentBrowserModule.Get().GetSelectedPathViewFolders(selectedFolders);

	if (selectedFolders.Num() == 0)
	{
		UE_LOG(KDUIPackerLog, Error, TEXT("** 请先选择一个文件夹 **"));
		return;
	}

	SelectedPath = selectedFolders[0];

	// 计算路径：DesignResource → SpriteSheet
	FString ContentRel = SelectedPath;
	if (ContentRel.StartsWith(TEXT("/Game/")))
	{
		ContentRel.RightChopInline(6);
	}
	InputDiskPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / ContentRel);

	// 从路径中提取 "UI/DesignResource/" 开始的内容，忽略 Sources Panel 可能加的 "All" 等前缀
	int32 DesignIdx = ContentRel.Find(TEXT("UI/DesignResource/"), ESearchCase::IgnoreCase);
	if (DesignIdx == INDEX_NONE)
	{
		// 回退：只要路径包含 DesignResource 就允许
		DesignIdx = ContentRel.Find(TEXT("DesignResource"), ESearchCase::IgnoreCase);
		if (DesignIdx == INDEX_NONE)
		{
			UE_LOG(KDUIPackerLog, Error, TEXT("** 请选择 DesignResource 目录下的文件夹，当前路径: %s **"), *ContentRel);
			State = EPackerProgress::None;
			return;
		}
	}
	// 截取 DesignResource 开始的部分作为干净路径
	FString CleanContentRel = ContentRel.RightChop(DesignIdx);
	InputDiskPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / CleanContentRel);
	OutputSpriteSheetRelPath = CleanContentRel.Replace(TEXT("DesignResource"), TEXT("SpriteSheet"), ESearchCase::IgnoreCase);

	// 记录选中文件夹名
	FString FolderName = FPaths::GetCleanFilename(SelectedPath).ToLower();
	TArray<FString> arr;
	FolderName.ParseIntoArray(arr, TEXT("_"), true);
	bIsDynamicFolder = (arr.Num() > 1 && arr[arr.Num() - 1].Find(TEXT("dyn")) == 0);

	SlowTask = MakeShared<FScopedSlowTask>(
		static_cast<int32>(EPackerProgress::End),
		LOCTEXT("UIPacker_Task", "打包图集...")
	);
	SlowTask->MakeDialog(true);

	UE_LOG(KDUIPackerLog, Warning, TEXT("===== InputDisk: %s, Output: /Game/%s ====="),
		*InputDiskPath, *OutputSpriteSheetRelPath);

	State = EPackerProgress::PreReload;

#endif
}

void FUIPackerModule::Tick(float DeltaTime)
{
	if (State == EPackerProgress::None)
	{
		return;
	}

	const FString OutputRelPath = GetSavedPath();
	const FString FileFolder = FPaths::ProjectContentDir() / OutputRelPath;
	const FString GameFileFolder = TEXT("/Game/") + OutputRelPath;
	const FString OutputDiskPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / OutputRelPath);
	const FString ScriptPath = GetScriptPath();

	if (State == EPackerProgress::PreReload)
	{
		if (SlowTask.IsValid()) SlowTask->EnterProgressFrame(1, LOCTEXT("PreReload", "准备打包..."));
		State = EPackerProgress::DeleteCheck;
	}

	else if (State == EPackerProgress::DeleteCheck)
	{
		if (SlowTask.IsValid()) SlowTask->EnterProgressFrame(1, LOCTEXT("DeleteCheck", "清理旧文件..."));
		UnLoadAndSavePackage(GameFileFolder);
		ShutdownWindows();

		FPlatformProcess::Sleep(1.0f);

		// 删除输出目录中的旧图集物理文件
		{
			IFileManager& FileManager = IFileManager::Get();
			FString AtlasPngPath = OutputDiskPath / TEXT("atlasTex.png");
			FString AtlasDataPath = OutputDiskPath / TEXT("atlas.paper2dsprites");
			if (FileManager.FileExists(*AtlasPngPath))
			{
				FileManager.Delete(*AtlasPngPath);
			}
			if (FileManager.FileExists(*AtlasDataPath))
			{
				FileManager.Delete(*AtlasDataPath);
			}
		}

		// 删除输出目录下的旧 UE 资产（Frames, Textures 子目录）
		DeleteNoUsedAssets();

		FPlatformProcess::Sleep(0.5f);
		State = EPackerProgress::Packer;
	}

	else if (State == EPackerProgress::Packer)
	{
		if (SlowTask.IsValid()) SlowTask->EnterProgressFrame(1, LOCTEXT("Packer", "打包图集中..."));
		// 确保输出目录存在
		IFileManager::Get().MakeDirectory(*OutputDiskPath, true);

		// 构建 Python 命令：设置 sys.argv + exec 执行 AutoPackUI.py
		FString PyCmd = FString::Printf(TEXT(
			"import sys\n"
			"sys.argv = [r'%s', r'%s', r'%s', '%s', '%s']\n"
			"exec(compile(open(r'%s', encoding='utf-8').read(), r'%s', 'exec'))"),
			*ScriptPath,      // sys.argv[0] = 脚本路径
			*InputDiskPath,   // sys.argv[1] = 输入目录（源 PNG 所在）
			*OutputDiskPath,  // sys.argv[2] = 输出目录（图集写入位置）
			bIsDynamicFolder ? TEXT("True") : TEXT("False"),  // sys.argv[3] = isDynamic
			TEXT("False"),    // sys.argv[4] = isMobile，默认 PC
			*ScriptPath,      // 要执行的脚本文件
			*ScriptPath);     // 文件名（用于错误栈）

		bool bSuc = IPythonScriptPlugin::Get()->ExecPythonCommand(*PyCmd);
		if (bSuc)
		{
			UE_LOG(KDUIPackerLog, Log, TEXT("** 打图集成功 **"));
			State = EPackerProgress::ImportAssets;
		}
		else
		{
			UE_LOG(KDUIPackerLog, Error, TEXT("** 打图集失败 **"));
			State = EPackerProgress::End;
		}
	}

	else if (State == EPackerProgress::ImportAssets)
	{
		if (SlowTask.IsValid()) SlowTask->EnterProgressFrame(1, LOCTEXT("Import", "导入图集..."));
		ImportAssets();
		State = EPackerProgress::ReloadAssets;
	}

	else if (State == EPackerProgress::ReloadAssets)
	{
		if (SlowTask.IsValid()) SlowTask->EnterProgressFrame(1, LOCTEXT("Reload", "重新加载..."));
		ReloadAssets();
		State = EPackerProgress::SaveFile;
	}

	else if (State == EPackerProgress::SaveFile)
	{
		if (SlowTask.IsValid()) SlowTask->EnterProgressFrame(1, LOCTEXT("Save", "保存文件..."));
		SaveAllFile(GameFileFolder, false);
		State = EPackerProgress::End;
	}

	else if (State == EPackerProgress::End)
	{
		SlowTask.Reset();
		SetTexQuality(ETextureCompressionQuality::TCQ_Highest);
		State = EPackerProgress::None;
	}
}

TStatId FUIPackerModule::GetStatId() const
{
	return TStatId();
}

bool FUIPackerModule::IsTickableInEditor() const
{
	return true;
}

void FUIPackerModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FUIPackerCommands::Get().PluginAction);
}

void FUIPackerModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FUIPackerCommands::Get().PluginAction);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUIPackerModule, UIPacker)
