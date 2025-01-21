// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "MAQuote.h"
#include "Modules/ModuleManager.h"

#include "Http.h"
#include "Json.h"
#include "Dom/JsonValue.h"
#include "Dom/JsonObject.h"

#include "Framework/Application/SlateApplication.h"

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
  void QuotesReset();

  bool CanDisplayQuote();
  void DisplayQuote();
  void CreateSlateWindow();
  void UpdateWindowQuote(const FString& _Quote, const FString & _Author);
  void KillWindow();

public: 
  FTSTicker::FDelegateHandle TickerHandle;
  TArray<MAQuote> Quotes;
  TSharedPtr<SWindow> SlateWindow;
  TSharedPtr<class MAQuoteWidget> WindowContent;
  bool bQuoteFetched_;
};
