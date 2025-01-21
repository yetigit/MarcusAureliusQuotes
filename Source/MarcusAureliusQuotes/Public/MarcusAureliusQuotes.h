// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "MAQuote.h"
#include "Modules/ModuleManager.h"

#include "Http.h"
#include "Json.h"
#include "Dom/JsonValue.h"
#include "Dom/JsonObject.h"

class FMarcusAureliusQuotesModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

  bool FetchQuotes();

  void OnResponseReceived(
      FHttpRequestPtr, FHttpResponsePtr, bool bWasSuccessful);
      
  bool Tick(float);

  void PrintAllQuotes(int Num, bool bFromBottom);
  void DisplayQuote();
  void QuotesReset();

public: 
  FTSTicker::FDelegateHandle TickerHandle;
  TArray<MAQuote> Quotes;
  bool bQuoteFetched_;
};
