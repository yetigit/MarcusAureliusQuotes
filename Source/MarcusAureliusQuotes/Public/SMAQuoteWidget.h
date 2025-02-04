#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SMAQuoteWidget : public SCompoundWidget
{
public:
  SLATE_BEGIN_ARGS(SMAQuoteWidget)
: 
	_DefaultWScreenSize(FVector2D::ZeroVector)
  {}
  SLATE_ARGUMENT(FVector2D, DefaultWScreenSize)
  SLATE_END_ARGS()

  void Construct(const FArguments& InArgs);
  void SetQuote(const FText& InQuote, const FText& InAuthor);
  void SetAuthorImg(const FSlateBrush * Img);
private:

  TSharedPtr<STextBlock> QuoteWidget;
  TSharedPtr<STextBlock> AuthorWidget;
public:
  TSharedPtr<SImage> AuthorImgWidget;

};
