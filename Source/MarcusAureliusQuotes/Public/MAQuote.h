#pragma once
#include "Containers/UnrealString.h"

class MAQuote
{
public:
  MAQuote() = default;
  MAQuote(FString const &InQuote, FString const &InAuthor) :
  quote(InQuote), author(InAuthor)
  {}
  FString quote;
  FString author;
};
