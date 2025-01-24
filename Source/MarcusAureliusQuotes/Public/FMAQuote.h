#pragma once
#include "Containers/UnrealString.h"

class FMAQuote
{
public:
  FMAQuote() = default;
  FMAQuote(FString const &InQuote, FString const &InAuthor) :
  quote(InQuote), author(InAuthor)
  {}
  FString quote;
  FString author;
};
