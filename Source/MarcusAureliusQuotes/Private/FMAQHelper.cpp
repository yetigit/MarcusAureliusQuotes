#include "FMAQHelper.h"
#include "Editor.h"

#include "HttpModule.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#include "SMAQuoteWidget.h"

#include "TimerManager.h"
#include "MarcusAureliusQuotesLog.h"

/*
 * NOTE:
 * binds:
 * http request
 * window to timer
 * ticker
*/

FMAQHelper::FMAQHelper() {
  QuoteTick_ = 8.f;
  WindowLifetime_ = QuoteTick_ * 0.7f;
  bQuoteFetched_ = false;
  NumQuotes_ = 99;
}

FMAQHelper::~FMAQHelper() { this->FreeResources(); }

void FMAQHelper::FreeResources() {
  if (GEditor) {
    GEditor->GetTimerManager()->ClearTimer(AutoHideTimerHandle);
  }
  KillWindow();
  QuotesReset();
  if (GEngine) {
    FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
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

void FMAQHelper::CreateSlateWindow() {

  TSharedPtr<SMAQuoteWidget> WindowContent = SNew(SMAQuoteWidget);
  WindowContentWP = WindowContent;
  TSharedPtr<SWindow> SlateWindow =
      SNew(SWindow)
          .Title(FText::FromString(TEXT("Quote")))
          .ClientSize(FVector2D(400, 400))
          .SupportsMaximize(false)
          .SupportsMinimize(false)

          .IsTopmostWindow(false)
          .FocusWhenFirstShown(false)
          .ActivationPolicy(EWindowActivationPolicy::Never)
          .IsPopupWindow(true)
          .ShouldPreserveAspectRatio(false)[WindowContent.ToSharedRef()];
  SlateWindowWP = SlateWindow;
  FSlateApplication::Get().AddWindow(SlateWindow.ToSharedRef());
}

void FMAQHelper::UpdateWindowQuote(const FString &_Quote,
                                   const FString &_Author) {
  auto SlateWindow = SlateWindowWP.Pin();
  auto WindowContent = WindowContentWP.Pin();
  if (SlateWindow.IsValid() && WindowContent.IsValid()) {
    WindowContent->SetQuote(FText::FromString(_Quote),
                            FText::FromString(_Author));
    FVector2D GoodSize = WindowContent->GetContentSize();
    SlateWindow->Resize(GoodSize);
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
        FString::Format(TEXT("{0}{1}{0}"), {"~", *Quote.author});

    UpdateWindowQuote(Quote.quote, AuthorPretty);

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

void FMAQHelper::AddTicker() {

  TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
      FTickerDelegate::CreateSP(AsShared(), &FMAQHelper::Tick), QuoteTick_);
}

bool FMAQHelper::Tick(float DeltaTime) {

  auto SlateWindow = SlateWindowWP.Pin();
  auto WindowContent = WindowContentWP.Pin();
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
  FString Url = {"https://stoic-quotes.com/api/quotes?num="};
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
