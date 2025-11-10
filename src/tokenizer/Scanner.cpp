#include <tokenizer/Scanner.hpp>
#include <iostream>

Scanner::Scanner(const std::string &source) : source(source) {}

bool Scanner::isAtEnd()
{
    if (current >= source.length())
        return true;
    return false;
}

char Scanner::advance()
{
    if (current >= source.length())
        return '\0';
    return source[current++];
}

char Scanner::peek()
{
    if (isAtEnd())
        return '\0';
    return source[current];
}

char Scanner::peekNext()
{
    if (current + 1 >= source.length())
        return '\0';
    return source[current + 1];
}

bool Scanner::match(char expected)
{
    if (isAtEnd() || source[current] != expected)
        return false;
    current++;
    return true;
}

void Scanner::addToken(TokenType type)
{
    addToken(type, std::any());
}

void Scanner::addToken(TokenType type, std::any literal)
{
    std::string text{source.substr(start, current - start)};
    tokens.emplace_back(type, text, literal, line);
}

bool Scanner::isDigit(char c)
{
    return (c >= '0' && c <= '9');
}

bool Scanner::isAlpha(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
}

bool Scanner::isAlphaNumeric(char c)
{
    return isDigit(c) || isAlpha(c);
}

void Scanner::number()
{
    while (isDigit(peek()))
        advance();

    if (peek() == '.' && isDigit(peekNext()))
    {
        advance();
        while (isDigit(peek()))
            advance();
    }

    std::string text{source.substr(start, current - start)};
    double number = std::stod(std::string{text});
    addToken(TokenType::NUMBER, number);
}

void Scanner::string()
{
    while (peek() != '"' && !isAtEnd())
        advance();

    if (peek() != '"')
    {
        throw std::runtime_error("string não fechada.");
    }

    advance();

    std::string text{source.substr(start + 1, current - start - 2)};
    addToken(TokenType::STRING, text);
}

void Scanner::identifier()
{
    while (isAlphaNumeric(peek()))
        advance();
    std::string text{source.substr(start, current - start)};
    auto it = keyWords.find(text);
    TokenType type = it == keyWords.end() ? TokenType::IDENTIFIER : it->second;
    addToken(type);
}

void Scanner::scanToken()
{
    char content = advance();

    switch (content)
    {
    // Caracteres simples
    case '(':
        addToken(TokenType::LEFT_PAREN);
        break;
    case ')':
        addToken(TokenType::RIGHT_PAREN);
        break;
    case '{':
        addToken(TokenType::LEFT_BRACE);
        break;
    case '}':
        addToken(TokenType::RIGHT_BRACE);
        break;
    case ',':
        addToken(TokenType::COMMA);
        break;
    case '.':
        addToken(TokenType::DOT);
        break;
    case '-':
        if (match('-'))
        {
            addToken(TokenType::MINUS_MINUS);
        }
        else
        {
            addToken(TokenType::MINUS);
        }
        break;
    case '+':
        if (match('+'))
        {
            addToken(TokenType::PLUS_PLUS);
        }
        else
        {
            addToken(TokenType::PLUS);
        }
        break;
    case '<':
        addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
        break;
    case '>':
        addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
        break;
    case '!':
        addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
        break;
    case ';':
        addToken(TokenType::SEMICOLON);
        break;
    case '*':
        addToken(TokenType::STAR);
        break;
    case '=':
        if (match('='))
        {
            addToken(TokenType::EQUAL_EQUAL);
        }
        else
        {
            addToken(TokenType::EQUAL);
        }
        break;
    case '/':
        if (match('/'))
        {
            // Comentário de linha - ignora até o fim
            while (peek() != '\n' && !isAtEnd())
                advance();
        }
        else if (match('*'))
        {
            while (!isAtEnd())
            {
                if (peek() == '*' && peekNext() == '/')
                {
                    advance(); // Consome o *
                    advance(); // Consome o /
                    break;
                }
                advance();
            }
        }
        else
        {
            addToken(TokenType::SLASH);
        }
        break;
    case '&':
        if (match('&'))
            addToken(TokenType::AND);
        break;
    case '|':
        if (match('|'))
            addToken(TokenType::OR);
        break;
    // Ignora whitespace
    case ' ':
    case '\r':
    case '\t':
        break;

    // Nova linha
    case '\n':
        line++;
        break;

    case '[':
        addToken(TokenType::LEFT_BRACKET);
        break;
    case ']':
        addToken(TokenType::RIGHT_BRACKET);
        break;
    case '"':
        string();
        break;

    default:
        if (isDigit(content))
        {
            number();
        }
        else if (isAlpha(content))
        {
            identifier();
        }
        else
        {
            throw std::runtime_error("caracter inesperado: '" + std::string(1, content) + "'");
        }
        break;
    }
}

std::vector<Token> Scanner::scanTokens()
{
    while (!isAtEnd())
    {
        start = current;
        scanToken();
    }
    tokens.emplace_back(TokenType::MONNY_EOF, "", std::any(), line);
    return tokens;
}
