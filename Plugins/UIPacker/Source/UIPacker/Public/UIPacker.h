// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(KDUIPackerLog, Log, All);

class FToolBarBuilder;
class FMenuBuilder;

enum class EPackerProgress
{
	None,
	PreReload,
	DeleteCheck,
    Packer,
    ImportAssets,
	ReloadAssets,
    SaveFile,    
	End
};

class FUIPackerModule : public IModuleInterface, public FTickableGameObject
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** This function will be bound to Command. */
	void PluginButtonClicked();	
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickableInEditor() const override;
	
private:
	FString SelectedPath;
	FString InputDiskPath;
	FString OutputSpriteSheetRelPath;
	TArray<FString> TargetPackageNames;
	bool bIsDynamicFolder = false;
	EPackerProgress State = EPackerProgress::None;

	void ShutdownWindows();
	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);
	FString GetSavedPath();
	void ImportAssets();
	void GetPackageNamesWithPaths(TArray<FString>& OutPackageNames, const FString& Path);
	bool CheckIsBpAsset(FName PackageName);
	void SaveAllFile(const FString& CS, bool reloadDirtyAsset);
	void ReloadAssets();
	void DeleteNoUsedAssets();
	void UnLoadAndSavePackage(const FString& CS);
	UAutomatedAssetImportData* SourceData;

	/**
	 * 设置 atlas 压缩质量，只针对 SpriteSheet 目录
	 */
	void SetTexQuality(ETextureCompressionQuality quality);

	/**
	 * 获取插件 Python 脚本的磁盘路径
	 */
	FString GetScriptPath();

private:
	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<struct FScopedSlowTask> SlowTask;
};
