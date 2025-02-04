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
  void QuotesReset();
  void InitQuoteTickers();



private:
  void CreateSlateWindow();
  void InitQuoteWindow(TSharedPtr<class ILevelEditor> InLevelEditor);


  bool Tick(float);
  void OnResponseReceived(FHttpRequestPtr, FHttpResponsePtr,
                          bool bWasSuccessful);

  bool CanDisplayQuote();
  void DisplayQuote();
  void UpdateWindowSize();
  void UpdateWindowQuote(const FString &_Quote, const FString &_Author);
  void KillWindow();

  void SetDefaults();
  void OnWorldTickStart(UWorld*, ELevelTick TickType, float DeltaTime);
  void TextureToBrush(const FString & _Path);
  void DestroyBrush();

private:
  FDelegateHandle MakeWindowTicker_;
  FTSTicker::FDelegateHandle QuoteTicker_;
  FDelegateHandle LevelVpPosDelegateHandle_;
  TArray<FMAQuote> Quotes;

  TWeakPtr<SWindow> SlateWindowWP;
  TWeakPtr<class SMAQuoteWidget> WindowContentWP;
  FTimerHandle AutoHideTimerHandle;


public:
  unsigned NumQuotes_;
  float QuoteTick_;
  float WindowLifetime_;
  FVector2D DefaultWindowPos_;
  bool bDefaultWindowPosSet_;
private:
  TSharedPtr<FSlateBrush> CachedAuthorImgBrush;
  TObjectPtr<UTexture2D> CachedTexture;
  bool bQuoteFetched_;
  bool bWindowWasEverCreated_;
};
