#include "../../include/tokenizer/Token.hpp"

Token::Token(TokenType type, const std::string &lexeme, const std::any &literal,
             int line)
    : type(type), lexeme(lexeme), literal(literal), line(line) {}

std::string Token::toString() {
  std::stringstream ss_literal;

  if (literal.has_value()) {
    const std::type_info &type = literal.type();

    if (type == typeid(std::string)) {
      ss_literal << std::any_cast<std::string>(literal);
    } else if (type == typeid(int)) {
      ss_literal << std::any_cast<int>(literal);
    } else if (type == typeid(double)) {
      ss_literal << std::any_cast<double>(literal);
    } else {
      ss_literal << "null";
    }
  } else {
    ss_literal << "[no literal]";
  }

  return lexeme + " " + ss_literal.str();
}
