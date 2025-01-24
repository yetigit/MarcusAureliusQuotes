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
* [] if the window is closed/destroyed upon quote display it must be recreated for the
quote display
* [] a picture of the author should appear with the quote and his name
* [] the window should not be separate from the editor in the taskbar
* [] progress bar for fetching quotes
* [x] use plugin log instead of LogTemp
* [x] auto size the window based on its content upon displayquote()
* [] better window size for the quote display
* [] add custom CLOSE button (which will hide the window only)
*/
// TODO :

// fix possible multi threading bugs with delegates, not passing this 
// but a shared pointer 

DEFINE_LOG_CATEGORY(LogMarcusAureliusQuotes);

#define LOCTEXT_NAMESPACE "FMarcusAureliusQuotesModule"

void FMarcusAureliusQuotesModule::StartupModule() {
  // This code will execute after your module is loaded into memory; the exact
  // timing is specified in the .uplugin file per-module

  ResourcesManager_ = MakeShared<FMAQHelper>();
  ResourcesManager_->QuotesReset();
  ResourcesManager_->CreateSlateWindow();

  ResourcesManager_->SetRequestTimeout(30.f);
  bool bSuccessfulFetch = ResourcesManager_->FetchQuotes();

  ResourcesManager_->AddTicker();

}

void FMarcusAureliusQuotesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
  
  ResourcesManager_->FreeResources();
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMarcusAureliusQuotesModule, MarcusAureliusQuotes)
