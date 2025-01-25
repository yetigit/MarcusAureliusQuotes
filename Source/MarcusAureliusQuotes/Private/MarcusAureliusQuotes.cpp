// Copyright Epic Games, Inc. All Rights Reserved.
#include "MarcusAureliusQuotes.h"
#include "Editor.h"
#include "FMAQHelper.h"

/*
* TODO:
* [x] the window must not focus when the quote gets displayed, but it must stay
on top without changing the focus
* [x] the window must disapear after X amount of seconds
* [x] the author text should lign up on the vertical
* [x] if the window is closed/destroyed upon quote display it must be recreated
for the quote display
* [] a picture of the author should appear with the quote and his name
* [x] the window should not be separate from the editor in the taskbar
* [] progress bar for fetching quotes
* [x] use plugin log instead of LogTemp
* [x] auto size the window based on its content upon displayquote()
* [x] better window size for the quote display
* [] add custom CLOSE button (which will hide the window only)
*/

#include "MarcusAureliusQuotesLog.h"

#define LOCTEXT_NAMESPACE "FMarcusAureliusQuotesModule"

void FMarcusAureliusQuotesModule::StartupModule() {
  // This code will execute after your module is loaded into memory; the exact
  // timing is specified in the .uplugin file per-module

  QuoteManager_ = MakeShared<FMAQHelper>();

  QuoteManager_->SetRequestTimeout(30.f);
  const float ticksEvery = 9.f;
  const float windowFadesAfter = ticksEvery * 0.7f;
  QuoteManager_->QuoteTick_ = ticksEvery;
  QuoteManager_->WindowLifetime_ = windowFadesAfter;
  QuoteManager_->NumQuotes_ = 99;

  QuoteManager_->QuotesReset();
  QuoteManager_->CreateSlateWindow();

  bool bSuccessfulFetch = QuoteManager_->FetchQuotes();

  QuoteManager_->AddTicker();
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
