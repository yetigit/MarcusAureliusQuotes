// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "MAQuote.h"
#include "Modules/ModuleManager.h"

#include "Http.h"
#include "Json.h"
#include "Dom/JsonObject.h"

class FMarcusAureliusQuotesModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

  void FetchQuotes();

  void OnResponseReceived(
      FHttpRequestPtr, FHttpResponsePtr, bool bWasSuccessful);
      
      

public: 
  TArray<MAQuote> Quotes;
};
