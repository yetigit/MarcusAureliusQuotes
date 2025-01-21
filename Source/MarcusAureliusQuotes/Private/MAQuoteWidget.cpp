#include "MAQuoteWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"

void MAQuoteWidget::Construct(const FArguments& InArgs)
{
  ChildSlot
  [
    SNew(SVerticalBox)
    +SVerticalBox::Slot()
    .Padding(FMargin(20, 10))
    .AutoHeight()
    [
      SNew(SVerticalBox)
      +SVerticalBox::Slot()
      [
        SAssignNew(QuoteWidget, STextBlock)
        .Text(FText::FromString(TEXT(
          "That which isn’t good for the hive, isn’t good for the bee."
        )))
        .AutoWrapText(true)
        .WrapTextAt(0.0f)
        .ColorAndOpacity(FLinearColor::White)
        .Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
        .Justification(ETextJustify::Center)
      ]
    ]
    +SVerticalBox::Slot()
    [

      SNew(SVerticalBox)
      +SVerticalBox::Slot()
      [
        SAssignNew(AuthorWidget, STextBlock)
        .Text(FText::FromString(TEXT(
          "Marcus Aurelius"
        )))
        .AutoWrapText(true)
        .WrapTextAt(0.0f)
        .ColorAndOpacity(FLinearColor::White)
        .Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
        .Justification(ETextJustify::Center)
      ]
    ]

  ];
}

void MAQuoteWidget::SetQuote(const FText& InQuote, const FText& InAuthor)
{
  if (QuoteWidget.IsValid() && AuthorWidget.IsValid()) {
    QuoteWidget->SetText(InQuote);
    AuthorWidget->SetText(InAuthor);
  }
}

