// Copyright Epic Games, Inc. All Rights Reserved.
#include "MarcusAureliusQuotes.h"

#include "HttpModule.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#define LOCTEXT_NAMESPACE "FMarcusAureliusQuotesModule"



void FMarcusAureliusQuotesModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FMarcusAureliusQuotesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FMarcusAureliusQuotesModule::FetchQuotes()
{
  unsigned NumberOfQuotes = 99;
  FString Url = {"https://stoic-quotes.com/api/quotes?num="};
  Url += NumberOfQuotes;

  TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request
  = FHttpModule::Get().CreateRequest();
  Request->SetVerb("GET");
  Request->SetURL(Url);


  TFunction<void(const TSharedPtr<FJsonObject>&)> OnSuccess = 
    [this](const TSharedPtr<FJsonObject>& Response){

    // TODO: this is a list of dictionnaries, we need to loop
    FString QuoteText;
    FString QuoteAuthor;
    MAQuote QuoteObject;
    if (Response->TryGetStringField("text", QuoteText)) {
      QuoteObject.quote = QuoteText;
    }

    if (Response->TryGetStringField("author", QuoteAuthor)) {
      QuoteObject.author = QuoteAuthor;
    }
    Quotes.Add(QuoteObject);

  };
  TFunction<void(const FString&)> OnError =
    [](const FString& InError){ UE_LOG(LogTemp, Error, TEXT("Error: %s"), *InError); };

  // TODO: does not match argument list
  // using FHttpRequestCompleteDelegate = TTSDelegate<void(FHttpRequestPtr
  // /*Request*/, FHttpResponsePtr /*Response*/, bool
  // /*bProcessedSuccessfully*/)>;
  Request->OnProcessRequestComplete().BindRaw(
    this, &FMarcusAureliusQuotesModule::OnResponseReceived, 
    OnSuccess, OnError);
  Request->ProcessRequest();
}


void FMarcusAureliusQuotesModule::OnResponseReceived(
    FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful,
    const TFunction<void(const TSharedPtr<FJsonObject> &)> &OnSuccess,
    const TFunction<void(const FString &)> &OnError){

    if (!Response.IsValid() || !bWasSuccessful)
    {
        OnError(TEXT("Request failed"));
        return;
    }

    if (Response->GetResponseCode() < 200 || 
    Response->GetResponseCode() > 299)
    {
        OnError(FString::Printf(TEXT("HTTP Error: %d"), 
                                Response->GetResponseCode()));
        return;
    }
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader =
        TJsonReaderFactory<>::Create(Response->GetContentAsString());

    if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
      OnSuccess(JsonObject);
    } else {
      OnError(TEXT("Failed to parse JSON response"));
    }
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMarcusAureliusQuotesModule, MarcusAureliusQuotes)
