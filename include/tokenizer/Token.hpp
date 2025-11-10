#pragma once

#include "TokenType.hpp"
#include <any>
#include <sstream>
#include <string>

class Token {
public:
  TokenType type;
  std::string lexeme;
  std::any literal;
  int line;

  Token(TokenType type, const std::string &, const std::any &, int);
  std::string toString();
};
