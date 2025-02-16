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

  void OnSettingsChanged(FName PropertyName);

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
  void OnWorldTickStart(UWorld *, ELevelTick TickType, float DeltaTime);

  void LoadAllAvatars();
  void TextureToBrush(const FString &_Path, TSharedPtr<FSlateBrush> &);

  bool CheckAllBrushes();
  bool BrushCheck(TSharedPtr<FSlateBrush> &);

  void DestroyAllBrushes();
  void DestroyBrush(TSharedPtr<FSlateBrush> &);
  // condition for the quote to display
  bool DoDisplay();

private:

  TMap<FName, TSharedPtr<FSlateBrush>> CachedBrushes_;
  TArray<TObjectPtr<UTexture2D>> CachedTextures_;

  TArray<FMAQuote> Quotes;

  FDelegateHandle MakeWindowTicker_;
  FTSTicker::FDelegateHandle QuoteTicker_;

  TWeakPtr<SWindow> SlateWindowWP;
  TWeakPtr<class SMAQuoteWidget> WindowContentWP;
  FTimerHandle AutoHideTimerHandle;

public:
  unsigned NumQuotes_;
  float QuoteTick_;
  float WindowLifetime_;
  float DisplayProbability;
  double ReqStartTime;
private:
  FTimerHandle SettingsUpdateTimerHandle;
  FVector2D DefaultWindowPos_;
  FVector2D InitVpSize_;
  bool bDefaultWindowPosSet_;
  bool bQuoteFetched_;
  bool bWindowWasEverCreated_;
public:
  bool bActive;
  bool bInGame;

};
