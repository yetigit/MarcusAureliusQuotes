#pragma once
#include "Containers/UnrealString.h"

class FMAQuote
{
public:
  FMAQuote() = default;


  FMAQuote(FMAQuote &&other) noexcept 
      : quote(MoveTemp(other.quote)), author(MoveTemp(other.author)) {}

  FMAQuote &operator=(FMAQuote &&other) noexcept {
    quote = MoveTemp(other.quote);
    author = MoveTemp(other.author);
    return *this;
  }

  FMAQuote(const FMAQuote &other) = default;
  FMAQuote &operator=(FMAQuote const & other) =default;

  FMAQuote(FString const &InQuote, FString const &InAuthor) :
  quote(InQuote), author(InAuthor)
  {}

  FMAQuote(FString &&InQuote, FString &&InAuthor) noexcept
      : quote(MoveTemp(InQuote)), author(MoveTemp(InAuthor)) {}


  FString quote;
  FString author;
};
