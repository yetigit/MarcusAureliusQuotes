// Copyright Epic Games, Inc. All Rights Reserved.
#include "MarcusAureliusQuotes.h"
#include "Editor.h"

#include "HttpModule.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"


#include "MAQuoteWidget.h"
// #include "Framework/Application/SlateApplication.h"
// #include "ViewportMessagePlugin.h"
// #include "Engine/GameEngine.h"
// #include "TimerManager.h"

#include "TimerManager.h"

/*
* TODO:
* [x] the window must not focus when the quote gets displayed, but it must stay
on top without changing the focus
* [x] the window must disapear after X amount of seconds
* [x] the author text should lign up on the vertical
* [] if the window is closed/destroyed upon quote display it must be recreated for the
quote display
* [] a picture of the author should appear with the quote and his name
* [x] the window should not be separate from the editor in the taskbar
* [] progress bar for fetching quotes
* [x] use plugin log instead of LogTemp
* [x] auto size the window based on its content upon displayquote()
* [] better window size for the quote display
* [] add custom CLOSE button (which will hide the window only)
*/

DEFINE_LOG_CATEGORY(LogMarcusAureliusQuotes);

#define LOCTEXT_NAMESPACE "FMarcusAureliusQuotesModule"

void FMarcusAureliusQuotesModule::CreateSlateWindow() {
    {
    TSharedPtr<MAQuoteWidget> WindowContentSP = SNew(MAQuoteWidget);
    WindowContent = WindowContentSP;
    TSharedPtr<SWindow> SlateWindowSP = SNew(SWindow)
        .Title(FText::FromString(TEXT("Quote")))
        .ClientSize(FVector2D(400, 400))
        .SupportsMaximize(false)
        .SupportsMinimize(false)

        .IsTopmostWindow(false)
        .FocusWhenFirstShown(false)
        .ActivationPolicy(EWindowActivationPolicy::Never)
        .IsPopupWindow(false)
        .ShouldPreserveAspectRatio(true)
        [WindowContentSP.ToSharedRef()];

    SlateWindow = SlateWindowSP;
    FSlateApplication::Get().AddWindow(SlateWindowSP.ToSharedRef());
}
    //SlateWindow.Pin()->DestroyWindowImmediately();
#if 0
  if (!SlateWindow.IsValid())
  {
      UE_LOG(LogTemp, Warning, TEXT("EXPECTED"));
	  check(0);
  }
  else {
	  check(0);
  }
#endif

}

void FMarcusAureliusQuotesModule::StartupModule() {
  // This code will execute after your module is loaded into memory; the exact
  // timing is specified in the .uplugin file per-module

  QuotesReset();
  CreateSlateWindow();

  FHttpModule::Get().SetHttpTimeout(30.0f);
  bool bSuccessfulFetch = FetchQuotes();

  QuoteTick_ = 10.f; // tick every x seconds
  WindowLifetime_ = QuoteTick_ * 0.75; // seconds as fraction of the tick
  TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
      FTickerDelegate::CreateRaw(this, &FMarcusAureliusQuotesModule::Tick),
      QuoteTick_);
}

void FMarcusAureliusQuotesModule::UpdateWindowQuote(const FString &_Quote,
                                                    const FString &_Author) {
    if (!SlateWindow.IsValid())
        return;
  if (SlateWindow.Pin() && WindowContent.Pin().IsValid()) {
      WindowContent.Pin()->SetQuote(FText::FromString(_Quote),
                            FText::FromString(_Author));
      FVector2D GoodSize = WindowContent.Pin()->GetContentSize();
      SlateWindow.Pin()->Resize(GoodSize);
  }
}
bool FMarcusAureliusQuotesModule::CanDisplayQuote()
{

  //int x = int(SlateWindow.IsValid());
  UE_LOG(LogMarcusAureliusQuotes, Warning, TEXT("CanDisplayQuote()... SlateWindow pointer validity = %p"), SlateWindow.Pin().Get());
  return SlateWindow.Pin().IsValid() && SlateWindow.Pin()->GetVisibility().IsVisible();
}

void FMarcusAureliusQuotesModule::DisplayQuote() {
  UE_LOG(LogMarcusAureliusQuotes, Warning, TEXT("GOING FOR DISPLAY"));
  int x = int(SlateWindow.IsValid());
  UE_LOG(LogMarcusAureliusQuotes, Warning, TEXT("SlateWindow pointer validity = %d"), x);

  if (bQuoteFetched_ && Quotes.Num() && CanDisplayQuote()) {
    MAQuote Quote = Quotes.Pop(true); // NOTE:Move opportunity

    // GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Green,);

    FString AuthorPretty =
        FString::Format(TEXT("{0}{1}{0}"), {"~", *Quote.author});

    UpdateWindowQuote(Quote.quote, AuthorPretty);

    if (SlateWindow.Pin().IsValid()) {

      SlateWindow.Pin()->ShowWindow();
       SlateWindow.Pin()->BringToFront(); // NOTE: if window is top-most this will happen on ShowWindow()

      GEditor->GetTimerManager()->ClearTimer(AutoHideTimerHandle);
      auto l_HideWindow = [this](){
        if(SlateWindow.Pin().IsValid())
        {
          SlateWindow.Pin()->HideWindow();
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

bool FMarcusAureliusQuotesModule::Tick(float DeltaTime) {
  if (GEngine) {
    if(CanDisplayQuote())
    {
      if(!bQuoteFetched_ && !FetchQuotes() && !bQuoteFetched_)
      {
          UE_LOG(LogMarcusAureliusQuotes, Warning, TEXT("Failed attempt to fetch quotes"));
          return true;
      }
      DisplayQuote();
    }
  }
  return true;
}

void FMarcusAureliusQuotesModule::PrintAllQuotes(int Num, bool bFromBottom) {

  if (bQuoteFetched_) {
    int len = FMath::Min(Quotes.Num(), Num);
    for (size_t i = 0; i < len; i++) {

      MAQuote Quote;
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
void FMarcusAureliusQuotesModule::QuotesReset()
{
  Quotes.Empty();
  bQuoteFetched_ = false;
}

void FMarcusAureliusQuotesModule::KillWindow() {
  if (SlateWindow.Pin().IsValid()) {
    SlateWindow.Pin()->RequestDestroyWindow();
    SlateWindow.Reset();
    if (WindowContent.Pin().IsValid()) {
      WindowContent.Reset();
    }
  }
}

void FMarcusAureliusQuotesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
  
  if (GEditor)
  {
    GEditor->GetTimerManager()->ClearTimer(AutoHideTimerHandle);
  }
  KillWindow();
  QuotesReset();
  if (GEngine)
  {
    FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
  }
}

bool FMarcusAureliusQuotesModule::FetchQuotes() {
  int NumberOfQuotes = 99;
  FString Url = {"https://stoic-quotes.com/api/quotes?num="};
  Url += FString::FromInt(NumberOfQuotes);

  UE_LOG(LogMarcusAureliusQuotes, Warning, TEXT("Fetching quotes.. ."));
  TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
      FHttpModule::Get().CreateRequest();
  Request->SetVerb("GET");
  Request->SetURL(Url);

  // using FHttpRequestCompleteDelegate = TTSDelegate<void(FHttpRequestPtr
  // /*Request*/, FHttpResponsePtr /*Response*/, bool
  // /*bProcessedSuccessfully*/)>;

  Request->OnProcessRequestComplete().BindRaw(
      this, &FMarcusAureliusQuotesModule::OnResponseReceived);

  return Request->ProcessRequest();
}

void FMarcusAureliusQuotesModule::OnResponseReceived(FHttpRequestPtr Request,
                                                     FHttpResponsePtr Response,
                                                     bool bWasSuccessful) {

  QuotesReset();

  auto OnError = [](const FString &InError) {
    UE_LOG(LogMarcusAureliusQuotes, Error, TEXT("Error: %s"), *InError);
  };

  auto OnSuccess = [this](const TArray<TSharedPtr<FJsonValue>> &JsonArray) {
    for (const auto &JsonVal : JsonArray) {
      auto &CurrentDict = JsonVal->AsObject();

      FString QuoteText;
      FString QuoteAuthor;
      MAQuote QuoteObject;

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

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMarcusAureliusQuotesModule, MarcusAureliusQuotes)
