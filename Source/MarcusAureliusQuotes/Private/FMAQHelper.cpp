#include "FMAQHelper.h"
#include "Editor.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "LevelEditor.h"
#include "SLevelViewport.h"

#include "HttpModule.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#include "SMAQuoteWidget.h"

#include "TimerManager.h"

#include "HAL/PlatformApplicationMisc.h"

#include "MarcusAureliusQuotesLog.h"
/*
 * NOTE:
 * binds:
 * http request
 * window to timer
 * ticker
*/

FMAQHelper::FMAQHelper() {
  this->SetDefaults();
}

void FMAQHelper::SetDefaults() {
  NumQuotes_ = 99;
  QuoteTick_ = 9.f;
  WindowLifetime_ = QuoteTick_ * 0.7f;
  DefaultWindowPos_ = FVector2D::ZeroVector;
  bDefaultWindowPosSet_ = false;
  bQuoteFetched_ = false;
  bWindowWasEverCreated_ = false;
}

FMAQHelper::~FMAQHelper() { this->FreeResources(); }

void FMAQHelper::FreeResources() {
  if (GEditor) {
    GEditor->GetTimerManager()->ClearTimer(AutoHideTimerHandle);
  }
  KillWindow();
  QuotesReset();
  if (GEngine) {
    FWorldDelegates::OnWorldTickStart.Remove(MakeWindowTicker_);
    FTSTicker::GetCoreTicker().RemoveTicker(QuoteTicker_);
  }
}

void FMAQHelper::KillWindow() {
  if (auto SlateWindow = SlateWindowWP.Pin()) {
    SlateWindow->RequestDestroyWindow();
    SlateWindow.Reset();
    if (auto WindowContent = WindowContentWP.Pin()) {
      WindowContent.Reset();
    }
  }
}

void FMAQHelper::OnWorldTickStart(UWorld*, ELevelTick TickType, float DeltaTime)
{
  if (GEditor && !GEditor->PlayWorld)
  {
    if(!bWindowWasEverCreated_)
    {
      FLevelEditorModule &LevelEditorModule =
        FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
      InitQuoteWindow(LevelEditorModule.GetFirstLevelEditor());
    }
  }
}

void FMAQHelper::InitQuoteWindow(TSharedPtr<ILevelEditor> InLevelEditor) {

  if (!InLevelEditor.IsValid())
    return;

  TSharedPtr<SLevelViewport> ActiveLevelViewport =
      InLevelEditor->GetActiveViewportInterface();
  if (!ActiveLevelViewport.IsValid())
    return;

  TWeakPtr<SViewport> ViewportWidgetWP =
      ActiveLevelViewport->GetViewportWidget();
  if (auto ViewportWidget = ViewportWidgetWP.Pin()) {
    const FGeometry &VPGeo = ViewportWidget->GetTickSpaceGeometry();
    const FVector2D GeoPosition = VPGeo.GetAbsolutePosition();
    const FVector2D GeoSize = VPGeo.GetAbsoluteSize();
    const FVector2D Padd(20, 20);
    float x = GeoPosition.X + GeoSize.X - Padd.X;
    float y = GeoPosition.Y + (GeoSize.Y / 2);
    FVector2D Result(x, y);
    FString ResultStr = Result.ToString();
#if 0
    UE_LOG(LogMarcusAureliusQuotes, Warning, TEXT("Viewport pos = %s"),
           *ResultStr);
#endif
    DefaultWindowPos_ = Result;
    bDefaultWindowPosSet_ = true;
    CreateSlateWindow();
  }
}

void FMAQHelper::CreateSlateWindow() {

  const float AppScale = FSlateApplication::Get().GetApplicationScale();
  const float DpiScaleFactor =
      FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(DefaultWindowPos_.X,
                                                         DefaultWindowPos_.Y);
#if 0
  UE_LOG(LogMarcusAureliusQuotes, Warning,
         TEXT("app scale = %f, dpi scale = %f"), AppScale, DpiScaleFactor);
#endif

  FVector2D DefaultScreenSize(530.f * AppScale, 600.f * AppScale);

  TSharedPtr<SMAQuoteWidget> WindowContent =
      SNew(SMAQuoteWidget).DefaultWScreenSize(DefaultScreenSize);
  WindowContentWP = WindowContent;

  TSharedPtr<SWindow> SlateWindow =
      SNew(SWindow)
          .Title(FText::FromString(TEXT("Quote")))
          .ClientSize(DefaultScreenSize)
		  .ScreenPosition({0., 0.})
          .SupportsMaximize(false)
          .SupportsMinimize(false)

          .IsTopmostWindow(false)
          .FocusWhenFirstShown(false)
          .ActivationPolicy(EWindowActivationPolicy::Never)
          .IsPopupWindow(false)
          .ShouldPreserveAspectRatio(false)[WindowContent.ToSharedRef()];
  SlateWindowWP = SlateWindow;
  // NOTE: you have to force it, unreal will place it in the center 
  // despite the position set in the construct args
  SlateWindow->MoveWindowTo(DefaultWindowPos_ - FVector2D(DefaultScreenSize.X, 0.));

  TSharedPtr<SWindow> ParentWindow =
      FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame")
          .GetParentWindow();
  if (ParentWindow.IsValid()) {
    FSlateApplication::Get().AddWindowAsNativeChild(
        SlateWindow.ToSharedRef(), ParentWindow.ToSharedRef(), false);
  } else {
    FSlateApplication::Get().AddWindow(SlateWindow.ToSharedRef(), false);
  }

  bWindowWasEverCreated_ = true;
}

void FMAQHelper::UpdateWindowSize() {

  if (auto SlateWindow = SlateWindowWP.Pin()) {

    const float DpiScaleFactor = SlateWindow->GetDPIScaleFactor();
    FVector2D WPos = SlateWindow->GetPositionInScreen();
    SlateWindow->SlatePrepass(
        FSlateApplicationBase::Get().GetApplicationScale() * DpiScaleFactor);
    const FVector2D WNewSize = SlateWindow->GetDesiredSize();
    FVector2D WRightBottomCorner = WPos + WNewSize;
    bool MoveIt = false;
    if(WRightBottomCorner.X > DefaultWindowPos_.X)
    {
      const float Offset = WRightBottomCorner.X - DefaultWindowPos_.X;
      WPos.X -= Offset;
      MoveIt = true;
    }

    SlateWindow->ReshapeWindow(WPos, WNewSize);
    if(MoveIt)
    {
      SlateWindow->SetCachedScreenPosition(WPos);
    }
    SlateWindow->SetCachedSize(WNewSize);

  }
}

void FMAQHelper::UpdateWindowQuote(const FString &_Quote,
                                   const FString &_Author) {
  auto SlateWindow = SlateWindowWP.Pin();
  auto WindowContent = WindowContentWP.Pin();
  if (SlateWindow.IsValid() && WindowContent.IsValid()) {
    WindowContent->SetQuote(FText::FromString(_Quote),
                            FText::FromString(_Author));
  }
}

bool FMAQHelper::CanDisplayQuote() {

  auto SlateWindow = SlateWindowWP.Pin();
  return SlateWindow.IsValid() && SlateWindow->GetVisibility().IsVisible();
}

void FMAQHelper::DisplayQuote() {

  if (bQuoteFetched_ && Quotes.Num() && CanDisplayQuote()) {

    auto SlateWindow = SlateWindowWP.Pin();
    auto WindowContent = WindowContentWP.Pin();
    FMAQuote && Quote = Quotes.Pop(true);

    FString AuthorPretty = 
        FString::Format(TEXT("{0} {1}"), {TEXT("──"), *Quote.author});

    UpdateWindowQuote(Quote.quote, AuthorPretty);
    UpdateWindowSize();
    if (SlateWindow.IsValid()) {

      SlateWindow->ShowWindow();

      // NOTE: if window is top-most this will
      // happen on ShowWindow()
      SlateWindow->BringToFront(); 

      GEditor->GetTimerManager()->ClearTimer(AutoHideTimerHandle);
      auto &SlateWindowWeak = this->SlateWindowWP;
      auto l_HideWindow = [SlateWindowWeak]() {
        if (auto p = SlateWindowWeak.Pin()) {
          p->HideWindow();
        }
      };
      GEditor->GetTimerManager()->SetTimer(
          AutoHideTimerHandle, FTimerDelegate::CreateLambda(l_HideWindow),
          WindowLifetime_, false);
    }

    if (!Quotes.Num()) {
      QuotesReset();
    }
  } else {
    UE_LOG(LogMarcusAureliusQuotes, Warning, TEXT("No quotes to display"));
  }
}

void FMAQHelper::InitQuoteTickers() {

  MakeWindowTicker_ = FWorldDelegates::OnWorldTickStart.AddSP(
      AsShared(), &FMAQHelper::OnWorldTickStart);

  QuoteTicker_ = FTSTicker::GetCoreTicker().AddTicker(
      FTickerDelegate::CreateSP(AsShared(), &FMAQHelper::Tick), QuoteTick_);
}

bool FMAQHelper::Tick(float DeltaTime) {


  if(!bWindowWasEverCreated_)
  {
    UE_LOG(LogMarcusAureliusQuotes, Error, TEXT("Failed to initialize window"));
    return true;
  }

  auto SlateWindow = SlateWindowWP.Pin();

  if(!SlateWindow.IsValid())
  {
    // Window has been destroyed, we re-create
    UE_LOG(LogMarcusAureliusQuotes, Warning, TEXT("WINDOW RECREATE"));
    CreateSlateWindow();
  }

  if (GEngine) {
    if (CanDisplayQuote()) {
      if (!bQuoteFetched_ && !FetchQuotes()) {
        UE_LOG(LogMarcusAureliusQuotes, Warning, TEXT("Failed attempt to fetch quotes"));
        return true;
      }
      DisplayQuote();
    }
  }
  return true;
}

void FMAQHelper::PrintAllQuotes(int Num, bool bFromBottom) {

  if (bQuoteFetched_) {
    int len = FMath::Min(Quotes.Num(), Num);
    for (size_t i = 0; i < len; i++) {

      FMAQuote Quote;
      if (bFromBottom) {
        Quote = Quotes[Quotes.Num() - i - 1];
      } else {
        Quote = Quotes[i];
      }
      const FString &currentQuote = Quote.quote;
      const FString &currentAuthor = Quote.author;

      UE_LOG(LogMarcusAureliusQuotes, Warning, TEXT("%s -- %s"), *currentQuote, *currentAuthor);
    }
  }
}
void FMAQHelper::QuotesReset() {
  Quotes.Empty();
  bQuoteFetched_ = false;
}

void FMAQHelper::SetRequestTimeout(const float _HowLong) {
  FHttpModule::Get().SetHttpTimeout(_HowLong);
}

bool FMAQHelper::FetchQuotes() {
  // int NumberOfQuotes = NumQuotes_;
  FString Url = TEXT("https://stoic-quotes.com/api/quotes?num=");
  Url += FString::FromInt(NumQuotes_);

  UE_LOG(LogMarcusAureliusQuotes, Warning, TEXT("Fetching quotes.. ."));
  TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
      FHttpModule::Get().CreateRequest();
  Request->SetVerb("GET");
  Request->SetURL(Url);

  Request->OnProcessRequestComplete().BindThreadSafeSP(this,
                                             &FMAQHelper::OnResponseReceived);

  return Request->ProcessRequest();
}

void FMAQHelper::OnResponseReceived(FHttpRequestPtr Request,
                                    FHttpResponsePtr Response,
                                    bool bWasSuccessful) {

  QuotesReset();

  auto OnError = [](const FString &InError) {
    UE_LOG(LogMarcusAureliusQuotes, Error, TEXT("Error: %s"), *InError);
  };

  auto OnSuccess = [this](const TArray<TSharedPtr<FJsonValue>> &JsonArray) {
    Quotes.Reserve(NumQuotes_);
    for (const auto &JsonVal : JsonArray) {
      auto &CurrentDict = JsonVal->AsObject();

      FString QuoteText;
      FString QuoteAuthor;
      FMAQuote QuoteObject;

      if (CurrentDict->TryGetStringField("text", QuoteText) &&
          CurrentDict->TryGetStringField("author", QuoteAuthor)) {
        QuoteObject.quote = QuoteText;
        QuoteObject.author = QuoteAuthor;
        Quotes.Add(QuoteObject);
      }
    }
    bQuoteFetched_ = bool(Quotes.Num());
  };

  if (!Response.IsValid() || !bWasSuccessful) {
    OnError(TEXT("Request failed"));
    return;
  }

  if (Response->GetResponseCode() < 200 || Response->GetResponseCode() > 299) {
    OnError(
        FString::Printf(TEXT("HTTP Error: %d"), Response->GetResponseCode()));
    return;
  }
  TArray<TSharedPtr<FJsonValue>> JsonArray;
  TSharedRef<TJsonReader<>> Reader =
      TJsonReaderFactory<>::Create(Response->GetContentAsString());

  if (FJsonSerializer::Deserialize(Reader, JsonArray)) {
    OnSuccess(JsonArray);
  } else {
    OnError(TEXT("Failed to parse JSON response"));
  }
}
