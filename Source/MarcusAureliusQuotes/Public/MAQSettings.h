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
    CategoryName = TEXT("Plugins"); 
    SectionName = TEXT("Stoic Quotes");
  }

  UPROPERTY(config, EditAnywhere, Category = "Quote Display", meta = (ClampMin = "0.0", ClampMax = "100.0", 
            UIMin = "0.0",
            UIMax = "100.0", Units = "%"))
  float DisplayProbability;

  UPROPERTY(config, EditAnywhere, Category = "Quote Display", meta = (ClampMin = "0.5", ClampMax = "60.0", 
            UIMin = "0.5",
            UIMax = "60.0", Units = "min"))
  float DisplayInterval;
};
