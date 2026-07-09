#include "GitOperations.h"

#include "Modules/ModuleManager.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "Misc/DateTime.h"
#include "AssetRegistry/AssetRegistryModule.h"

DEFINE_LOG_CATEGORY_STATIC(LogKDDGitHelper, Log, All);

FString FGitOperations::MakeFileLabel(const TArray<FString>& InFiles)
{
	if (InFiles.Num() == 0)
	{
		return FString();
	}
	FString First = FPaths::GetCleanFilename(InFiles[0]);
	if (InFiles.Num() > 1)
	{
		First += FString::Printf(TEXT(" (等 %d 个文件)"), InFiles.Num());
	}
	return First;
}

FGitCommandLog FGitOperations::MakeFailLog(const FString& InCommand, const FString& InDetail, const FString& InMessage)
{
	FGitCommandLog Log;
	Log.Time = FDateTime::Now();
	Log.Command = InCommand;
	Log.bSuccess = false;
	Log.Detail = InDetail;
	Log.Message = InMessage;
	return Log;
}

// 从常见安装路径查找 git.exe
static FString FindGitViaCommonPaths()
{
	TArray<FString> Candidates;
	Candidates.Add(TEXT("D:/Git/bin/git.exe"));
	Candidates.Add(TEXT("D:/Git/cmd/git.exe"));
	Candidates.Add(TEXT("C:/Program Files/Git/bin/git.exe"));
	Candidates.Add(TEXT("C:/Program Files/Git/cmd/git.exe"));
	Candidates.Add(TEXT("C:/Program Files (x86)/Git/bin/git.exe"));
	for (const FString& Path : Candidates)
	{
		if (FPaths::FileExists(Path))
		{
			return Path;
		}
	}
	return FString();
}

FString FGitOperations::FindGitExecutable()
{
	FString Path = FindGitViaCommonPaths();
	if (!Path.IsEmpty())
	{
		return Path;
	}
	return TEXT("git");
}

FString FGitOperations::FindRepoRoot(const FString& InFromFile)
{
	FString Dir = FPaths::GetPath(InFromFile);
	for (int32 i = 0; i < 20; ++i)
	{
		if (Dir.IsEmpty())
		{
			break;
		}
		const FString GitDir = FPaths::Combine(Dir, TEXT(".git"));
		if (FPaths::DirectoryExists(GitDir))
		{
			return Dir;
		}
		const FString Parent = FPaths::GetPath(Dir);
		if (Parent == Dir)
		{
			break;
		}
		Dir = Parent;
	}
	return FString();
}

TArray<FString> FGitOperations::ResolveAssetFilePaths(const TArray<FAssetData>& InAssets)
{
	TArray<FString> Paths;
	for (const FAssetData& Asset : InAssets)
	{
		const FString PackageName = Asset.PackageName.ToString();
		if (PackageName.IsEmpty())
		{
			continue;
		}
		// 自动检测扩展名：先查 .umap（地图），不存在则用 .uasset（普通资产）
		const FString MapFilePath = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetMapPackageExtension());
		if (FPaths::FileExists(MapFilePath))
		{
			Paths.Add(MapFilePath);
		}
		else
		{
			const FString FilePath = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
			Paths.Add(FilePath);
		}
	}
	return Paths;
}

bool FGitOperations::RunGitCommand(const FString& InGitPath, const TArray<FString>& InArgs, const FString& InWorkingDir, FString& OutSummary)
{
	FString CommandLine;
	for (int32 i = 0; i < InArgs.Num(); ++i)
	{
		if (i > 0)
		{
			CommandLine += TEXT(" ");
		}
		CommandLine += FString::Printf(TEXT("\"%s\""), *InArgs[i]);
	}

	UE_LOG(LogKDDGitHelper, Log, TEXT("执行: %s %s (工作目录: %s)"), *InGitPath, *CommandLine, *InWorkingDir);

	void* ReadPipe = nullptr;
	void* WritePipe = nullptr;
	if (!FPlatformProcess::CreatePipe(ReadPipe, WritePipe))
	{
		OutSummary = TEXT("无法创建进程管道");
		return false;
	}

	FProcHandle ProcHandle = FPlatformProcess::CreateProc(
		*InGitPath,
		*CommandLine,
		false, true, true,
		nullptr, 0,
		*InWorkingDir,
		WritePipe,
		nullptr);

	if (!ProcHandle.IsValid())
	{
		FPlatformProcess::ClosePipe(ReadPipe, WritePipe);
		OutSummary = TEXT("无法启动 git 进程");
		return false;
	}

	FString FullOutput;
	const double TimeoutSeconds = 120.0;
	const double StartTime = FPlatformTime::Seconds();
	bool bTimedOut = false;
	while (FPlatformProcess::IsProcRunning(ProcHandle))
	{
		const FString NewLine = FPlatformProcess::ReadPipe(ReadPipe);
		if (!NewLine.IsEmpty())
		{
			FullOutput += NewLine;
			UE_LOG(LogKDDGitHelper, Log, TEXT("%s"), *NewLine);
		}
		if (FPlatformTime::Seconds() - StartTime > TimeoutSeconds)
		{
			UE_LOG(LogKDDGitHelper, Warning, TEXT("git 命令超时，终止进程。"));
			FPlatformProcess::TerminateProc(ProcHandle, true);
			bTimedOut = true;
			break;
		}
		FPlatformProcess::Sleep(0.05f);
	}

	const FString Remaining = FPlatformProcess::ReadPipe(ReadPipe);
	if (!Remaining.IsEmpty())
	{
		FullOutput += Remaining;
		UE_LOG(LogKDDGitHelper, Log, TEXT("%s"), *Remaining);
	}

	if (bTimedOut)
	{
		FPlatformProcess::CloseProc(ProcHandle);
		FPlatformProcess::ClosePipe(ReadPipe, WritePipe);
		OutSummary = TEXT("[超时] git 命令被终止");
		return false;
	}

	int32 ReturnCode = -1;
	FPlatformProcess::GetProcReturnCode(ProcHandle, &ReturnCode);
	FPlatformProcess::CloseProc(ProcHandle);
	FPlatformProcess::ClosePipe(ReadPipe, WritePipe);

	OutSummary = FullOutput;
	return ReturnCode == 0;
}

TArray<FGitCommandLog> FGitOperations::UpdateLogged(const TArray<FString>& InFiles)
{
	TArray<FGitCommandLog> Logs;
	if (InFiles.Num() == 0)
	{
		Logs.Add(MakeFailLog(TEXT("git fetch"), TEXT("没有指定任何文件"), TEXT("未指定文件")));
		return Logs;
	}
	const FString Git = FindGitExecutable();
	const FString RepoRoot = FindRepoRoot(InFiles[0]);
	if (RepoRoot.IsEmpty())
	{
		Logs.Add(MakeFailLog(TEXT("git rev-parse --show-toplevel"), TEXT("未找到 Git 仓库根目录"), TEXT("未找到 Git 仓库")));
		return Logs;
	}

	// 1) 刷新远程引用
	TArray<FString> FetchArgs;
	FetchArgs.Add(TEXT("fetch"));
	Logs.Add(RunGitCommandLogged(Git, FetchArgs, RepoRoot, TEXT("git fetch")));
	if (!Logs.Last().bSuccess)
	{
		return Logs;
	}

	// 2) 取上游跟踪分支
	FString Upstream;
	TArray<FString> RefArgs;
	RefArgs.Add(TEXT("rev-parse"));
	RefArgs.Add(TEXT("--abbrev-ref"));
	RefArgs.Add(TEXT("@{upstream}"));
	FString UpstreamOutput;
	if (!RunGitCommand(Git, RefArgs, RepoRoot, UpstreamOutput))
	{
		Logs.Add(MakeFailLog(TEXT("git rev-parse @{upstream}"), TEXT("无法获取上游分支"), TEXT("未配置上游分支")));
		return Logs;
	}
	Upstream = UpstreamOutput.TrimStartAndEnd();
	if (Upstream.IsEmpty())
	{
		Logs.Add(MakeFailLog(TEXT("git rev-parse @{upstream}"), TEXT("当前分支未配置上游跟踪分支"), TEXT("未配置上游分支")));
		return Logs;
	}

	// 3) 更新为上游版本
	TArray<FString> Args;
	Args.Add(TEXT("checkout"));
	Args.Add(Upstream);
	Args.Add(TEXT("--"));
	Args.Append(InFiles);
	const FString Label = FString::Printf(TEXT("git checkout %s -- %s"), *Upstream, *MakeFileLabel(InFiles));
	Logs.Add(RunGitCommandLogged(Git, Args, RepoRoot, Label));
	return Logs;
}

TArray<FGitCommandLog> FGitOperations::CheckoutVersionLogged(const TArray<FString>& InFiles, const FString& InCommitHash)
{
	TArray<FGitCommandLog> Logs;
	if (InFiles.Num() == 0)
	{
		Logs.Add(MakeFailLog(TEXT("git checkout"), TEXT("没有指定任何文件"), TEXT("未指定文件")));
		return Logs;
	}
	const FString Git = FindGitExecutable();
	const FString RepoRoot = FindRepoRoot(InFiles[0]);
	if (RepoRoot.IsEmpty())
	{
		Logs.Add(MakeFailLog(TEXT("git rev-parse --show-toplevel"), TEXT("未找到 Git 仓库根目录"), TEXT("未找到 Git 仓库")));
		return Logs;
	}
	TArray<FString> Args;
	Args.Add(TEXT("checkout"));
	Args.Add(InCommitHash);
	Args.Add(TEXT("--"));
	Args.Append(InFiles);
	const FString Label = FString::Printf(TEXT("git checkout %s -- %s"), *InCommitHash.Left(8), *MakeFileLabel(InFiles));
	Logs.Add(RunGitCommandLogged(Git, Args, RepoRoot, Label));
	return Logs;
}

bool FGitOperations::GetFileHistory(const TArray<FString>& InFiles, TArray<FGitFileHistoryEntry>& OutHistory)
{
	if (InFiles.Num() == 0)
	{
		return false;
	}
	const FString Git = FindGitExecutable();
	const FString RepoRoot = FindRepoRoot(InFiles[0]);
	if (RepoRoot.IsEmpty())
	{
		return false;
	}
	TArray<FString> Args;
	Args.Add(TEXT("log"));
	Args.Add(TEXT("--pretty=format:%H%x1f%h%x1f%an%x1f%ad%x1f%s%x1e"));
	Args.Add(TEXT("--date=short"));
	Args.Add(TEXT("--"));
	Args.Append(InFiles);

	FString Raw;
	if (!RunGitCommand(Git, Args, RepoRoot, Raw))
	{
		return false;
	}

	TArray<FString> Records;
	Raw.ParseIntoArray(Records, TEXT("\x1e"), true);
	for (const FString& Rec : Records)
	{
		TArray<FString> Fields;
		Rec.ParseIntoArray(Fields, TEXT("\x1f"), false);
		if (Fields.Num() < 5)
		{
			continue;
		}
		FGitFileHistoryEntry Entry;
		Entry.Hash = Fields[0];
		Entry.ShortHash = Fields[1];
		Entry.Author = Fields[2];
		Entry.Date = Fields[3];
		Entry.Subject = Fields[4];
		OutHistory.Add(Entry);
	}
	return true;
}

FGitCommandLog FGitOperations::RunGitCommandLogged(const FString& InGitPath, const TArray<FString>& InArgs, const FString& InWorkingDir, const FString& InCommandLabel)
{
	FGitCommandLog Log;
	Log.Time = FDateTime::Now();
	Log.Command = InCommandLabel;

	FString Summary;
	const bool bOk = RunGitCommand(InGitPath, InArgs, InWorkingDir, Summary);
	Log.bSuccess = bOk;
	Log.Detail = Summary;

	FString Msg = Summary;
	Msg.ReplaceInline(TEXT("\r"), TEXT(""));
	if (Msg.Contains(TEXT("\n")))
	{
		Msg = Msg.Left(Msg.Find(TEXT("\n")));
	}
	Msg.TrimStartAndEndInline();
	if (Msg.IsEmpty())
	{
		Msg = bOk ? TEXT("成功") : TEXT("失败");
	}
	if (Msg.Len() > 300)
	{
		Msg = Msg.Left(300) + TEXT("...");
	}
	Log.Message = Msg;
	return Log;
}

void FGitOperations::RescanFiles(const TArray<FString>& Files)
{
	if (FModuleManager::Get().IsModuleLoaded(TEXT("AssetRegistry")))
	{
		FAssetRegistryModule& ARM = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		ARM.Get().ScanFilesSynchronous(Files, /*bForceRescan=*/ true);
	}
}
