// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"
#include "Modules/ModuleManager.h"

#include "Templates/SharedPointer.h"


DECLARE_LOG_CATEGORY_EXTERN(LogMarcusAureliusQuotes, Log, All);


class FMarcusAureliusQuotesModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

  TSharedPtr<class FMAQHelper> ResourcesManager_;
};
