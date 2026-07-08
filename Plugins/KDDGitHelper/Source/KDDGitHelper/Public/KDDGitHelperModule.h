#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "AssetRegistry/AssetData.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Commands/UICommandList.h"
#include "GitOperations.h"

class FKDDGitHelperModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	// 获取模块实例
	static FKDDGitHelperModule& GetModule()
	{
		return FModuleManager::LoadModuleChecked<FKDDGitHelperModule>("KDDGitHelper");
	}

private:
	// 注册 Content Browser 右键菜单扩展
	void RegisterContentBrowserContextMenu();

	// Content Browser 资产选中时的菜单扩展回调
	TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets);

	// 右键命令：Git 更新
	void ExecuteUpdate(TArray<FAssetData> SelectedAssets);

	// 右键命令：Git 历史
	void ExecuteHistory(TArray<FAssetData> SelectedAssets);

	// 还原后强制重载资产
	void ReloadRevertedAssets(const TArray<FAssetData>& SelectedAssets);
};
