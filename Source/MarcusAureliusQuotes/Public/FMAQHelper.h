#pragma once

#include "CoreMinimal.h"
#include "FMAQuote.h"
#include "Templates/SharedPointer.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Http.h"
#include "Json.h"

#include "Framework/Application/SlateApplication.h"

class FMAQHelper : public TSharedFromThis<FMAQHelper> {
public:
  FMAQHelper();
  ~FMAQHelper();
  void FreeResources();
  void SetRequestTimeout(const float);

public:
  bool FetchQuotes();
  void PrintAllQuotes(int Num, bool bFromBottom);
  void CreateSlateWindow();
  void QuotesReset();
  void AddTicker();

private:
  bool Tick(float);
  void OnResponseReceived(FHttpRequestPtr, FHttpResponsePtr,
                          bool bWasSuccessful);

  bool CanDisplayQuote();
  void DisplayQuote();
  void UpdateWindowQuote(const FString &_Quote, const FString &_Author);
  void KillWindow();

private:
  FTSTicker::FDelegateHandle TickerHandle;
  TArray<FMAQuote> Quotes;

  TWeakPtr<SWindow> SlateWindowWP;
  TWeakPtr<class SMAQuoteWidget> WindowContentWP;
  FTimerHandle AutoHideTimerHandle;

  float QuoteTick_;
  float WindowLifetime_;
  bool bQuoteFetched_;
};
