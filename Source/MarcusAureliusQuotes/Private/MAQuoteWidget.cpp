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
    .VAlign(VAlign_Center)
    .HAlign(HAlign_Fill)
    .Padding(FMargin(10, 30, 10, 5))
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
    +SVerticalBox::Slot()
    .AutoHeight()
    .VAlign(VAlign_Top)
    .HAlign(HAlign_Center)
    .Padding(FMargin(0, 0, 0, 40))
    [
      SAssignNew(AuthorWidget, STextBlock)
      .Text(FText::FromString(TEXT(
        "Marcus Aurelius"
      )))
      .ColorAndOpacity(FLinearColor::White)
      .Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
      .Justification(ETextJustify::Center)
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

