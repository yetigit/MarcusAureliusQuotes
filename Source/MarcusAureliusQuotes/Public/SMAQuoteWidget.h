// Copyright Baidhir Hidair, 2025. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SMAQuoteWidget : public SCompoundWidget
{
public:
  SLATE_BEGIN_ARGS(SMAQuoteWidget)
: 
	_DefaultWScreenSize(FVector2D::ZeroVector),
	_DefaultAuthorImgRes(FSlateApplication::Get().GetApplicationScale() * 64.0f)
  {}
  SLATE_ARGUMENT(FVector2D, DefaultWScreenSize)
  SLATE_ARGUMENT(float, DefaultAuthorImgRes)
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
