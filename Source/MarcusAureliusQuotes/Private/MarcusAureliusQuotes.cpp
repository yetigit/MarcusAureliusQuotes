// Copyright Epic Games, Inc. All Rights Reserved.
#include "MarcusAureliusQuotes.h"
#include "Editor.h"
#include "FMAQHelper.h"

/*
 * TODO:
 * [x] a picture of the author should appear with the quote and his name
 * [] add brushes for seneca and epictetus
 * [] try loading the plugin on default loading phase
 * [] Attempt to change the plugin name
 */

#include "MarcusAureliusQuotesLog.h"

#define LOCTEXT_NAMESPACE "FMarcusAureliusQuotesModule"

void FMarcusAureliusQuotesModule::StartupModule() {
  // This code will execute after your module is loaded into memory; the exact
  // timing is specified in the .uplugin file per-module

  QuoteManager_ = MakeShared<FMAQHelper>();

  QuoteManager_->QuotesReset();

  QuoteManager_->NumQuotes_ = 99;
  QuoteManager_->SetRequestTimeout(30.f);
  if (!QuoteManager_->FetchQuotes())
  {
    UE_LOG(LogMarcusAureliusQuotes, Error, TEXT("Failed to fetch quotes"));
  }

  
  QuoteManager_->InitQuoteTickers();

}

void FMarcusAureliusQuotesModule::ShutdownModule() {
  // This function may be called during shutdown to clean up your module.  For
  // modules that support dynamic reloading, we call this function before
  // unloading the module.

  QuoteManager_->FreeResources();
  if (QuoteManager_.IsValid()) {
    QuoteManager_.Reset();
  }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMarcusAureliusQuotesModule, MarcusAureliusQuotes)
