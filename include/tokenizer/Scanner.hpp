#pragma once

#include "Token.hpp"
#include <string>
#include <vector>
#include <unordered_map>

class Scanner {
private:
  std::string source;
  size_t current = 0;
  size_t start = 0;
  int line;
  std::vector<Token> tokens;

  bool isAtEnd();
  void scanToken();
  char advance();
  char peek();
  char peekNext();
  bool match(char expected);

  void addToken(TokenType type);
  void addToken(TokenType type, std::any literal);

  bool isDigit(char c);
  bool isAlpha(char c);
  bool isAlphaNumeric(char c);

  void number();
  void string();
  void identifier();

  std::unordered_map<std::string, TokenType> keyWords = {
    {"print", TokenType::PRINT},
    {"for", TokenType::FOR},
    {"while", TokenType::WHILE},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"include", TokenType::INCLUDE},
    {"for", TokenType::FOR},
    {"def", TokenType::DEF},
    {"func", TokenType::FUNC},
    {"for", TokenType::FOR},
    {"nil", TokenType::NIL}, // todo
    {"return", TokenType::RETURN},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"to_string", TokenType::TO_STRING},
    {"input", TokenType::INPUT},
    {"to_number", TokenType::TO_NUMBER},
    {"clear", TokenType::CLEAR},
    {"const", TokenType::CONST}
  };

public:
  Scanner(const std::string &);
  std::vector<Token> scanTokens();
};
