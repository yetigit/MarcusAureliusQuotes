#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class MAQuoteWidget : public SCompoundWidget
{
public:
  SLATE_BEGIN_ARGS(MAQuoteWidget)
  {}
  SLATE_END_ARGS()

  void Construct(const FArguments& InArgs);
  void SetQuote(const FText& InQuote, const FText& InAuthor);
  FVector2D GetContentSize() const;
private:

  TSharedPtr<STextBlock> QuoteWidget;
  TSharedPtr<STextBlock> AuthorWidget;

};
