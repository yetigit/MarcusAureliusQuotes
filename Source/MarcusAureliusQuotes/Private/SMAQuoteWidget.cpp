#include "SMAQuoteWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "MarcusAureliusQuotesLog.h"

void SMAQuoteWidget::Construct(const FArguments& InArgs)
{
  const FVector2D IntendedScreenSize = InArgs._DefaultWScreenSize;
  const float AppScale = FSlateApplication::Get().GetApplicationScale();
  ChildSlot
  [
    SNew(SVerticalBox)
    +SVerticalBox::Slot()
    .AutoHeight()
    .VAlign(VAlign_Fill)
    .HAlign(HAlign_Center)
    .Padding(FMargin(10. * AppScale, 10., 10. * AppScale, 10.))
    [
      SAssignNew(QuoteWidget, STextBlock)
      .Text(FText::FromString(TEXT(
        "That which isn’t good for the hive, isn’t good for the bee."
      )))
	  .WrapTextAt(IntendedScreenSize.X)
      .AutoWrapText(true)
      .ColorAndOpacity(FLinearColor::White)
      .Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
      .Justification(ETextJustify::Center)
    ]
    +SVerticalBox::Slot()
    .AutoHeight()
    .VAlign(VAlign_Fill)
    .HAlign(HAlign_Center)
    .Padding(FMargin(0., 0., 0., 20.))
    [
      SAssignNew(AuthorWidget, STextBlock)
      .Text(FText::FromString(TEXT(
        "── Marcus Aurelius")))
      .ColorAndOpacity(FLinearColor::White)
      .Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
      .Justification(ETextJustify::Center)
    ]

  ];
}

void SMAQuoteWidget::SetQuote(const FText& InQuote, const FText& InAuthor)
{
  if (QuoteWidget.IsValid() && AuthorWidget.IsValid()) {
    QuoteWidget->SetText(InQuote);
    AuthorWidget->SetText(InAuthor);
  }
}

