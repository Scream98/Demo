// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeGameDemo_init() {}
static_assert(!UE_WITH_CONSTINIT_UOBJECT, "This generated code can only be compiled with !UE_WITH_CONSTINIT_OBJECT");
	static FPackageRegistrationInfo Z_Registration_Info_UPackage__Script_GameDemo;
	FORCENOINLINE UPackage* Z_Construct_UPackage__Script_GameDemo(ETypeConstructPhase)
	{
		if (!Z_Registration_Info_UPackage__Script_GameDemo.OuterSingleton)
		{
		static const UECodeGen_Private::FPackageParams PackageParams = {
			"/Script/GameDemo",
			nullptr,
			0,
			PKG_CompiledIn | 0x00000000,
			0xAD364A37,
			0x0BE40488,
			METADATA_PARAMS(0, nullptr)
		};
		UECodeGen_Private::ConstructUPackage(Z_Registration_Info_UPackage__Script_GameDemo.OuterSingleton, PackageParams);
	}
	return Z_Registration_Info_UPackage__Script_GameDemo.OuterSingleton;
}
static FRegisterCompiledInInfo Z_CompiledInDeferPackage_UPackage__Script_GameDemo(Z_Construct_UPackage__Script_GameDemo, TEXT("/Script/GameDemo"), Z_Registration_Info_UPackage__Script_GameDemo, CONSTRUCT_RELOAD_VERSION_INFO(FPackageReloadVersionInfo, 0xAD364A37, 0x0BE40488));
PRAGMA_ENABLE_DEPRECATION_WARNINGS
