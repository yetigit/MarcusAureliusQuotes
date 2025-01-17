// Copyright Epic Games, Inc. All Rights Reserved.
#include "MarcusAureliusQuotes.h"

#include "HttpModule.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#define LOCTEXT_NAMESPACE "FMarcusAureliusQuotesModule"



void FMarcusAureliusQuotesModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

  FHttpModule::Get().SetHttpTimeout(30.0f);
}

void FMarcusAureliusQuotesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FMarcusAureliusQuotesModule::FetchQuotes()
{
  int NumberOfQuotes = 99;
  FString Url = {"https://stoic-quotes.com/api/quotes?num="};
  Url += FString::FromInt(NumberOfQuotes);

  TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request
  = FHttpModule::Get().CreateRequest();
  Request->SetVerb("GET");
  Request->SetURL(Url);

  // using FHttpRequestCompleteDelegate = TTSDelegate<void(FHttpRequestPtr
  // /*Request*/, FHttpResponsePtr /*Response*/, bool
  // /*bProcessedSuccessfully*/)>;
  
  Request->OnProcessRequestComplete().BindRaw(
    this, &FMarcusAureliusQuotesModule::OnResponseReceived);
  Request->ProcessRequest();
}

void FMarcusAureliusQuotesModule::OnResponseReceived(FHttpRequestPtr Request,
                                                     FHttpResponsePtr Response,
                                                     bool bWasSuccessful) {

  auto OnError = [](const FString &InError) {
    UE_LOG(LogTemp, Error, TEXT("Error: %s"), *InError);
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
