// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#include "MyBlankProgram.h"

#include "RequiredProgramMainCPPInclude.h"

#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY_STATIC(LogMyBlankProgram, Log, All);

IMPLEMENT_APPLICATION(BlankProgram, "MyBlankProgram");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
#if defined(ParentProjectName)
	const FString ProjectDir = FString::Printf(TEXT("../../../%s/Programs/%s/"), ParentProjectName, FApp::GetProjectName());
	FPlatformMisc::SetOverrideProjectDir(ProjectDir);
#endif

	GEngineLoop.PreInit(ArgC, ArgV);
	UE_LOG(LogMyBlankProgram, Display, TEXT("Hello World"));

	return 0;
}
