#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SMAQuoteWidget : public SCompoundWidget
{
public:
  SLATE_BEGIN_ARGS(SMAQuoteWidget)
  {}
  SLATE_END_ARGS()

  void Construct(const FArguments& InArgs);
  void SetQuote(const FText& InQuote, const FText& InAuthor);
  FVector2D GetQuotationSize(TWeakPtr<SWindow> _Host) const;
private:

  TSharedPtr<STextBlock> QuoteWidget;
  TSharedPtr<STextBlock> AuthorWidget;

};
