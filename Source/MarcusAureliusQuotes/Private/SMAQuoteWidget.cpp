#include "SMAQuoteWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"

void SMAQuoteWidget::Construct(const FArguments& InArgs)
{
  ChildSlot
  [
    SNew(SVerticalBox)
    +SVerticalBox::Slot()
    .AutoHeight()
    .VAlign(VAlign_Top)
    .HAlign(HAlign_Fill)
    .Padding(FMargin(0, 0, 0, 0))
    [
      SAssignNew(QuoteWidget, STextBlock)
      .Text(FText::FromString(TEXT(
        "That which isn’t good for the hive, isn’t good for the bee."
      )))
      .AutoWrapText(true)
      .WrapTextAt(0.f)
      .ColorAndOpacity(FLinearColor::White)
      .Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
      .Justification(ETextJustify::Center)
    ]
    +SVerticalBox::Slot()
    .MinHeight(30.0f)
    .AutoHeight()
    .VAlign(VAlign_Top)
    .HAlign(HAlign_Fill)
    .Padding(FMargin(0, 0, 0, 0))
    [
      SAssignNew(AuthorWidget, STextBlock)
      .Text(FText::FromString(TEXT(
        "-- Marcus Aurelius"
      )))
      .ColorAndOpacity(FLinearColor::White)
      .Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
      .Justification(ETextJustify::Center)
    ]

  ];
}
FVector2D SMAQuoteWidget::GetQuotationSize() const
{
  FVector2D MinSize(100, 125);
  FVector2D Margin(15, 30);

  if(QuoteWidget.IsValid() && AuthorWidget.IsValid())
  {
    FVector2D wh;
    FVector2D QuoteWSize = QuoteWidget->GetDesiredSize();
    FVector2D AuthorWSize = AuthorWidget->GetDesiredSize();

    float TotalHeight = QuoteWSize.Y + AuthorWSize.Y;

    wh.X = FMath::Max(MinSize.X, FMath::Max(QuoteWSize.X, AuthorWSize.X));
    wh.Y = FMath::Max(MinSize.Y, TotalHeight);

    wh += Margin;
    return wh;
  }
  return MinSize;
}

void SMAQuoteWidget::SetQuote(const FText& InQuote, const FText& InAuthor)
{
  if (QuoteWidget.IsValid() && AuthorWidget.IsValid()) {
    QuoteWidget->SetText(InQuote);
    AuthorWidget->SetText(InAuthor);
  }
}

