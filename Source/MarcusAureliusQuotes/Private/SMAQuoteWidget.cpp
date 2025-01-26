#include "SMAQuoteWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "MarcusAureliusQuotesLog.h"

void SMAQuoteWidget::Construct(const FArguments& InArgs)
{
  ChildSlot
  [
    SNew(SVerticalBox)
    +SVerticalBox::Slot()
    .FillHeight(3.0)
    .VAlign(VAlign_Fill)
    .HAlign(HAlign_Fill)
    .Padding(FMargin(0, 0, 0, 0))
    [
      SAssignNew(QuoteWidget, STextBlock)
      .Text(FText::FromString(TEXT(
        "That which isn’t good for the hive, isn’t good for the bee."
      )))
      .AutoWrapText(true)
      .ColorAndOpacity(FLinearColor::White)
      .Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
      .Justification(ETextJustify::Center)
    ]
    +SVerticalBox::Slot()
    .FillHeight(1.0)
    .VAlign(VAlign_Fill)
    .HAlign(HAlign_Center)
    .Padding(FMargin(0, 20, 0, 0))
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
FVector2D SMAQuoteWidget::GetQuotationSize(TWeakPtr<SWindow> _Host) const
{

  if(auto HostSP = _Host.Pin())
  {
    FVector2D HostSize = HostSP->GetDesiredSize();
    FString HostSizeStr = HostSP->GetDesiredSize().ToString();
    UE_LOG(LogMarcusAureliusQuotes, Warning, TEXT("Host Size = %s"), *HostSizeStr);
    return HostSize + FVector2D(0.f, 40.f);
  }
  return FVector2D::ZeroVector;

  #if 0

  float TitleBarHeight = 0.0f;
  if(auto HostSP = _Host.Pin())
  {
    auto TitleBarSize = HostSP->GetTitleBarSize();
    if (TitleBarSize.IsSet())
      TitleBarHeight = TitleBarSize.Get();
  }
	TSharedPtr<SVerticalBox> VBoxWidget = StaticCastSharedPtr<SVerticalBox>(QuoteWidget->GetParentWidget());
    FVector2D VBoxSize = VBoxWidget->GetDesiredSize();
    VBoxSize.Y += TitleBarHeight;
    FString BoxSizeStr = VBoxSize.ToString();
    UE_LOG(LogMarcusAureliusQuotes, Warning, TEXT("Final Size = %s"), *BoxSizeStr);
	return VBoxSize;
  #endif
}

void SMAQuoteWidget::SetQuote(const FText& InQuote, const FText& InAuthor)
{
  if (QuoteWidget.IsValid() && AuthorWidget.IsValid()) {
    QuoteWidget->SetText(InQuote);
    AuthorWidget->SetText(InAuthor);
  }
}

