// Copyright Baidhir Hidair, 2025. All Rights Reserved.
#include "MarcusAureliusQuotes.h"
#include "Editor.h"
#include "FMAQHelper.h"
#include "MAQSettings.h"
#include "ISettingsModule.h"


#include "MarcusAureliusQuotesLog.h"

#define LOCTEXT_NAMESPACE "FMarcusAureliusQuotesModule"

void FMarcusAureliusQuotesModule::StartupModule() {
  // This code will execute after your module is loaded into memory; the exact
  // timing is specified in the .uplugin file per-module
  if (ISettingsModule *SettingsModule =
          FModuleManager::GetModulePtr<ISettingsModule>("Settings")) {
    SettingsModule->RegisterSettings(
        "Project", "Plugins", "Stoic Quotes",
        LOCTEXT("RuntimeSettingsName", "Stoic Quotes"),
        LOCTEXT("RuntimeSettingsDescription",
                "Configure Stoic quotes plugin"),
        GetMutableDefault<UMAQSettings>());
  }

  LogMarcusAureliusQuotes.SetVerbosity(ELogVerbosity::Error);


  QuoteManager_ = MakeShared<FMAQHelper>();

  QuoteManager_->QuotesReset();

  QuoteManager_->NumQuotes_ = 99;
  QuoteManager_->SetRequestTimeout(30.f);
  if (!QuoteManager_->FetchQuotes())
  {
    UE_LOG(LogMarcusAureliusQuotes, Error, TEXT("Failed to fetch quotes"));
  }

  const UMAQSettings* Settings = GetDefault<UMAQSettings>  () ; 
  check(Settings);
  QuoteManager_->DisplayProbability = Settings->DisplayProbability / 100.f ;
  QuoteManager_->bActive = Settings->bActive  ;
  QuoteManager_->bInGame = Settings->bInGame  ;

  // display interval is in minute
  const float QuoteTick = Settings->DisplayInterval * 60.f;
  QuoteManager_->QuoteTick_ = QuoteTick;
  // seconds read minimum
  QuoteManager_->WindowLifetime_ = 12.0f;
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
  // NOTE: must unregister settings
  if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
  {
    SettingsModule->UnregisterSettings("Project", "Plugins", "Stoic Quotes");
  }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMarcusAureliusQuotesModule, MarcusAureliusQuotes)
