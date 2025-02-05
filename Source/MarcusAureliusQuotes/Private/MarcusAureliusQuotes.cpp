// Copyright Epic Games, Inc. All Rights Reserved.
#include "MarcusAureliusQuotes.h"
#include "Editor.h"
#include "FMAQHelper.h"

/*
 * TODO:
 * [x] Attempt to change the plugin name
 * [x] Implement display of quote based on probability per minute
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

  
  const float QuoteTick = 60.f;
  QuoteManager_->DisplayProbability = 1/4.f;
  QuoteManager_->QuoteTick_ = QuoteTick;
  QuoteManager_->WindowLifetime_ = QuoteTick * 0.5f;
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
