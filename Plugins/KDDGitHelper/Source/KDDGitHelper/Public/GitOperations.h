#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"

// 单条 git 命令的执行日志（用于统一结果弹窗）
struct FGitCommandLog
{
	FDateTime Time;        // 执行时刻
	FString Command;       // 命令符，如 "git fetch"
	FString Message;       // 列表里显示的简短消息
	FString Detail;       // 完整输出，点击行时显示
	bool bSuccess = false;
};

// 文件历史记录中的单条提交
struct FGitFileHistoryEntry
{
	FString Hash;          // 完整 commit hash
	FString ShortHash;     // 短 hash
	FString Author;        // 提交者
	FString Date;          // 提交日期（YYYY-MM-DD）
	FString Subject;       // 提交说明（首行）
};

// Git 操作工具类
class KDDGITHELPER_API FGitOperations
{
public:
	// 定位可用 git.exe
	static FString FindGitExecutable();

	// 从某文件向上查找包含 .git 的仓库根目录
	static FString FindRepoRoot(const FString& InFromFile);

	// 由选中的 FAssetData 列表解析出对应的物理文件路径（自动处理 .uasset / .umap）
	static TArray<FString> ResolveAssetFilePaths(const TArray<FAssetData>& InAssets);

	// 执行一条 git 命令
	static bool RunGitCommand(const FString& InGitPath, const TArray<FString>& InArgs, const FString& InWorkingDir, FString& OutSummary);

	// 单文件更新（fetch + checkout 上游），返回多步日志
	static TArray<FGitCommandLog> UpdateLogged(const TArray<FString>& InFiles);

	// 拉取指定历史版本到本地（git checkout <commit> -- <files>），返回日志
	static TArray<FGitCommandLog> CheckoutVersionLogged(const TArray<FString>& InFiles, const FString& InCommitHash);

	// 取文件提交历史
	static bool GetFileHistory(const TArray<FString>& InFiles, TArray<FGitFileHistoryEntry>& OutHistory);

	// 运行单条命令并返回结构化日志
	static FGitCommandLog RunGitCommandLogged(const FString& InGitPath, const TArray<FString>& InArgs, const FString& InWorkingDir, const FString& InCommandLabel);

	// 强制资产注册表重新扫描文件
	static void RescanFiles(const TArray<FString>& Files);

private:
	// 生成文件标签
	static FString MakeFileLabel(const TArray<FString>& InFiles);

	// 构造失败日志
	static FGitCommandLog MakeFailLog(const FString& InCommand, const FString& InDetail, const FString& InMessage);
};
