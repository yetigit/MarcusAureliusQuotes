// Copyright Epic Games, Inc. All Rights Reserved.
#include "MarcusAureliusQuotes.h"

#include "HttpModule.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"


// #include "ViewportMessagePlugin.h"
// #include "Engine/GameEngine.h"
// #include "TimerManager.h"

#define LOCTEXT_NAMESPACE "FMarcusAureliusQuotesModule"

void FMarcusAureliusQuotesModule::StartupModule() {
  // This code will execute after your module is loaded into memory; the exact
  // timing is specified in the .uplugin file per-module

  QuotesReset();
  FHttpModule::Get().SetHttpTimeout(30.0f);
  bool bQuoteFetched = FetchQuotes();

  const float QuoteTick = 5.f;
  TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
      FTickerDelegate::CreateRaw(this, &FMarcusAureliusQuotesModule::Tick),
      QuoteTick);
}

void FMarcusAureliusQuotesModule::DisplayQuote() {
  if (bQuoteFetched_) {

    if (Quotes.Num()) {

      MAQuote Quote = Quotes.Pop(true);
      FString quotation = Quote.quote;
      quotation += (FString("\n--") + Quote.author);

      GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Green, quotation);

      if (!Quotes.Num()) {
        QuotesReset();
      }
    }
  } else {
    UE_LOG(LogTemp, Warning, TEXT("No quotes, going for refill"));
    QuotesReset();
    FetchQuotes();
    // GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Green, TEXT("No quotes"));
  }
}

bool FMarcusAureliusQuotesModule::Tick(float DeltaTime) {
  if (GEngine) {
    DisplayQuote();
  }
  return true;
}

void FMarcusAureliusQuotesModule::PrintAllQuotes(int Num, bool bFromBottom) {

  if (bQuoteFetched_ && Quotes.Num() >= Num) {
    for (size_t i = 0; i < Num; i++) {

      MAQuote Quote;
      if (bFromBottom) {
        Quote = Quotes[Quotes.Num() - i - 1];
      } else {
        Quote = Quotes[i];
      }
      const FString &currentQuote = Quote.quote;
      const FString &currentAuthor = Quote.author;

      UE_LOG(LogTemp, Warning, TEXT("%s -- %s"), *currentQuote, *currentAuthor);
    }
  }
}
void FMarcusAureliusQuotesModule::QuotesReset()
{
  Quotes.Empty();
  bQuoteFetched_ = false;
}

void FMarcusAureliusQuotesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
  
  QuotesReset();
  if (GEngine)
  {
    FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
  }
}

bool FMarcusAureliusQuotesModule::FetchQuotes() {
  int NumberOfQuotes = 5;
  FString Url = {"https://stoic-quotes.com/api/quotes?num="};
  Url += FString::FromInt(NumberOfQuotes);

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

  auto OnError = [](const FString &InError) {
    UE_LOG(LogTemp, Error, TEXT("Error: %s"), *InError);
  };

  auto OnSuccess = [this](const TArray<TSharedPtr<FJsonValue>> &JsonArray) {
    Quotes.Empty();
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
    bQuoteFetched_ = true;
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
