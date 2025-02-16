#pragma once


#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MAQSettings.generated.h"

UCLASS(config = EditorPerProjectUserSettings,
       defaultconfig, meta = (DisplayName = "Stoic Quotes"))
class MARCUSAURELIUSQUOTES_API UMAQSettings : public UDeveloperSettings
{
  GENERATED_BODY()

public:
  UMAQSettings()
  {
    DisplayProbability = 50.0f;
    DisplayInterval = 1.0f;
    bActive = true;
    bInGame = false;
    CategoryName = TEXT("Plugins"); 
    SectionName = TEXT("Stoic Quotes");
  }

  // Called after a property is modified
  virtual void
  PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override {
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // Get the name of the property that was changed
    FName PropertyName = (PropertyChangedEvent.Property != nullptr)
                             ? PropertyChangedEvent.Property->GetFName()
                             : NAME_None;
    OnSettingsChanged.Broadcast(PropertyName);
  }

  UPROPERTY(config, EditAnywhere, Category = "Quote Display", meta = (ClampMin = "0.0", ClampMax = "100.0", 
            UIMin = "0.0",
            UIMax = "100.0", Units = "%"))
  float DisplayProbability;

  UPROPERTY(config, EditAnywhere, Category = "Quote Display", meta = (ClampMin = "0.5", ClampMax = "60.0", 
            UIMin = "0.5",
            UIMax = "60.0", Units = "min"))
  float DisplayInterval;

  UPROPERTY(config, EditAnywhere, Category = "Quote Display", meta = (EditCondition="true"))
  bool bActive;

  UPROPERTY(config, EditAnywhere, Category = "Quote Display", meta = (EditCondition="true"))
  bool bInGame;

  DECLARE_MULTICAST_DELEGATE_OneParam(FOnSettingsChanged, FName /*PropertyName*/);
  FOnSettingsChanged OnSettingsChanged;

};
