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
      FHttpRequestPtr, FHttpResponsePtr, bool bWasSuccessful,
      const TFunction<void(const TSharedPtr<FJsonObject> &)> &OnSuccess,
      const TFunction<void(const FString &)> &OnError);

public: 
  TArray<MAQuote> Quotes;
};
