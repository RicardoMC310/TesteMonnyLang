#include <parser/Parser.hpp>
#include <parser/Expr.hpp>
#include <parser/Stmt.hpp>
#include <iostream>
#include <stdexcept>

Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens) {}

std::vector<std::shared_ptr<Statements::Stmt>> Parser::parse()
{
    std::vector<std::shared_ptr<Statements::Stmt>> statements;

    while (!isAtEnd())
    {
        statements.push_back(statement());
    }

    return statements;
}

std::vector<std::shared_ptr<Statements::Stmt>> Parser::block()
{
    std::vector<std::shared_ptr<Statements::Stmt>> statements;

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
    {
        statements.push_back(statement());
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return statements;
}

std::shared_ptr<Statements::Stmt> Parser::statement()
{
    if (peek().type == TokenType::PLUS_PLUS || peek().type == TokenType::MINUS_MINUS)
    {
        return incrementStatement();
    }

    if (match(TokenType::IF))
    {
        return ifStatement();
    }
    if (match(TokenType::PRINT))
    {
        return printStatement();
    }
    if (match(TokenType::DEF))
    {
        return varStatement();
    }
    if (match(TokenType::WHILE))
    {
        return whileStatement();
    }
    if (match(TokenType::FOR))
    {
        return forStatement();
    }
    if (match(TokenType::LEFT_BRACE))
    {
        auto statements = block();
        return std::make_shared<Statements::Block>(statements);
    }
    if (match(TokenType::CLEAR))
        return clearStatement();
    if (match(TokenType::FUNC))
        return functionStatement();
    if (match(TokenType::CONST))
    { // NOVO
        return constStatement();
    }
    return expressionStatement();
}

std::shared_ptr<Statements::Const> Parser::constStatement() {
    Token name = consume(TokenType::IDENTIFIER, "Expected constant name");

    // CONST deve ter inicializador obrigatório
    consume(TokenType::EQUAL, "Constants must be initialized with '='");
    
    std::shared_ptr<Expr> initializer = expression();

    consume(TokenType::SEMICOLON, "Expect ';' after constant value.");
    return std::make_shared<Statements::Const>(name, initializer);
}

std::shared_ptr<Statements::FunctionDef> Parser::functionStatement()
{
    Token name = consume(TokenType::IDENTIFIER, "Expected function name");

    consume(TokenType::LEFT_PAREN, "Expected '(' after function name.");

    std::vector<Token> params;
    if (!check(TokenType::RIGHT_PAREN))
    {
        do
        {
            params.push_back(consume(TokenType::IDENTIFIER, "Expect parameter name"));
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RIGHT_PAREN, "Expected ')' after function params.");
    consume(TokenType::LEFT_BRACE, "Expected '{' before function body.");

    auto bodyStatements = block();
    auto body = std::make_shared<Statements::Block>(bodyStatements);

    return std::make_shared<Statements::FunctionDef>(name, params, body);
}

std::shared_ptr<Statements::Clear> Parser::clearStatement()
{
    consume(TokenType::LEFT_PAREN, "Expected '(' after clear");
    consume(TokenType::RIGHT_PAREN, "Expected ')' after clear");
    consume(TokenType::SEMICOLON, "Expected ';' after ')'");
    return std::make_shared<Statements::Clear>();
}

std::shared_ptr<Statements::Stmt> Parser::incrementStatement()
{
    Token oper = previous(); // ++ ou --

    // DEVE ser seguido por um identificador
    Token identifier = consume(TokenType::IDENTIFIER, "Expect variable name after '++' or '--'");
    consume(TokenType::SEMICOLON, "Expect ';' after increment.");

    auto var = std::make_shared<Variable>(identifier);
    auto increment = std::make_shared<Increment>(oper, var, true); // prefix
    return std::make_shared<Statements::Expression>(increment);
}

std::shared_ptr<Statements::While> Parser::whileStatement()
{
    consume(TokenType::LEFT_PAREN, "Expected '(' after while.");

    std::shared_ptr<Expr> expr = expression();

    consume(TokenType::RIGHT_PAREN, "Expected ')' after condition.");

    std::shared_ptr<Statements::Stmt> body = statement();

    return std::make_shared<Statements::While>(expr, body);
}

std::shared_ptr<Statements::Stmt> Parser::forStatement()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");

    // 1. INICIALIZAÇÃO
    std::shared_ptr<Statements::Stmt> initializer;
    if (match(TokenType::SEMICOLON))
    {
        // for (; condição; incremento) - inicialização vazia
        initializer = nullptr;
    }
    else if (match(TokenType::DEF))
    {
        // for (def i = 0; condição; incremento)
        initializer = varStatement();
    }
    else
    {
        // for (i = 0; condição; incremento)
        initializer = expressionStatement();
    }

    // 2. CONDIÇÃO
    std::shared_ptr<Expr> condition = nullptr;
    if (!check(TokenType::SEMICOLON))
    {
        condition = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

    // 3. INCREMENTO
    std::shared_ptr<Expr> increment = nullptr;
    if (!check(TokenType::RIGHT_PAREN))
    {
        increment = expression();
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

    // 4. CORPO
    std::shared_ptr<Statements::Stmt> body = statement();

    // ========== TRANSFORMAÇÃO FOR → WHILE ==========

    // Se tem incremento, adiciona ele no final do corpo
    if (increment != nullptr)
    {
        // Cria um bloco com: corpo + incremento
        std::vector<std::shared_ptr<Statements::Stmt>> newBodyStatements;

        // Adiciona o corpo original
        if (auto bodyBlock = std::dynamic_pointer_cast<Statements::Block>(body))
        {
            // Se o corpo já é um bloco, adiciona todas as statements
            newBodyStatements = bodyBlock->statements;
        }
        else
        {
            // Se o corpo é uma única statement, adiciona ela
            newBodyStatements.push_back(body);
        }

        // Adiciona o incremento no final do bloco
        newBodyStatements.push_back(std::make_shared<Statements::Expression>(increment));

        body = std::make_shared<Statements::Block>(newBodyStatements);
    }

    // Se não tem condição, usa true (loop infinito)
    if (condition == nullptr)
    {
        condition = std::make_shared<Literal>(true);
    }

    // Cria o while: inicialização; while (condição) { corpo }
    std::shared_ptr<Statements::Stmt> whileLoop =
        std::make_shared<Statements::While>(condition, body);

    // Se tem inicialização, cria um bloco com: inicialização + while
    if (initializer != nullptr)
    {
        std::vector<std::shared_ptr<Statements::Stmt>> statements;
        statements.push_back(initializer);
        statements.push_back(whileLoop);
        return std::make_shared<Statements::Block>(statements);
    }

    return whileLoop;
}

std::shared_ptr<Statements::IF> Parser::ifStatement()
{
    consume(TokenType::LEFT_PAREN, "Expected '(' after if");
    std::shared_ptr<Expr> condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after condition");

    std::shared_ptr<Statements::Stmt> thenStatement = statement();

    std::shared_ptr<Statements::Stmt> elseStatement = nullptr;

    if (match(TokenType::ELSE))
    {
        elseStatement = statement();
    }

    return std::make_shared<Statements::IF>(condition, thenStatement, elseStatement);
}

std::shared_ptr<Statements::Var> Parser::varStatement()
{
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name");

    std::shared_ptr<Expr> initializer = nullptr;
    if (match(TokenType::EQUAL))
    {
        initializer = expression();
    }

    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    return std::make_shared<Statements::Var>(name, initializer);
}

std::shared_ptr<Statements::Print> Parser::printStatement()
{
    consume(TokenType::LEFT_PAREN, "Expected '( after print.");

    std::vector<std::shared_ptr<Expr>> expressions;

    if (!check(TokenType::RIGHT_PAREN))
    {
        do
        {
            expressions.push_back(expression());
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
    consume(TokenType::SEMICOLON, "Expect ';' after value.");

    return std::make_shared<Statements::Print>(expressions);
}

std::shared_ptr<Statements::Expression> Parser::expressionStatement()
{
    std::shared_ptr<Expr> expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_shared<Statements::Expression>(expr);
}

std::shared_ptr<Expr> Parser::expression()
{
    return assignment();
}

std::shared_ptr<Expr> Parser::assignment()
{
    std::shared_ptr<Expr> expr = logicalOr();

    if (match(TokenType::PLUS_PLUS, TokenType::MINUS_MINUS))
    {
        Token oper = previous();

        if (auto var = std::dynamic_pointer_cast<Variable>(expr))
        {
            return std::make_shared<Increment>(oper, var, false);
        }

        // Suporte para arrays: arr[0]++
        if (auto arrayAccess = std::dynamic_pointer_cast<ArrayAccess>(expr))
        {
            // Implementação para incremento em array (mais complexa)
            throw std::runtime_error("Array increment not yet implemented");
        }

        throw std::runtime_error("Invalid increment target");
    }

    if (match(TokenType::EQUAL))
    {
        Token equals = previous();
        std::shared_ptr<Expr> value = assignment();

        if (auto var = std::dynamic_pointer_cast<Variable>(expr))
        {
            Token name = var->name;
            return std::make_shared<Assign>(name, value);
        }

        if (auto arrayAccess = std::dynamic_pointer_cast<ArrayAccess>(expr))
        {
            return std::make_shared<ArrayAssign>(arrayAccess->array, arrayAccess->index, value);
        }

        throw std::runtime_error("Invalid assignment target.");
    }

    return expr;
}

std::shared_ptr<Expr> Parser::logicalOr()
{
    std::shared_ptr<Expr> expr = logicalAnd();

    while (match(TokenType::OR))
    {
        Token oper = previous();
        std::shared_ptr<Expr> right = logicalAnd();
        expr = std::make_shared<Logical>(expr, oper, right);
    }

    return expr;
}

std::shared_ptr<Expr> Parser::logicalAnd()
{
    std::shared_ptr<Expr> expr = equality();

    while (match(TokenType::AND))
    {
        Token oper = previous();
        std::shared_ptr<Expr> right = equality();
        expr = std::make_shared<Logical>(expr, oper, right);
    }

    return expr;
}

std::shared_ptr<Expr> Parser::equality()
{
    std::shared_ptr<Expr> expr = comparison();

    while (match(TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL))
    {
        Token oper = previous();
        std::shared_ptr<Expr> right = comparison();
        expr = std::make_shared<Binary>(expr, oper, right);
    }

    return expr;
}

std::shared_ptr<Expr> Parser::comparison()
{
    std::shared_ptr<Expr> expr = addition();

    while (match(TokenType::GREATER, TokenType::GREATER_EQUAL,
                 TokenType::LESS, TokenType::LESS_EQUAL))
    {
        Token oper = previous();
        std::shared_ptr<Expr> right = addition();
        expr = std::make_shared<Binary>(expr, oper, right);
    }

    return expr;
}

std::shared_ptr<Expr> Parser::addition()
{
    std::shared_ptr<Expr> expr = multiplication();

    while (match(TokenType::PLUS, TokenType::MINUS))
    {
        Token oper = previous();
        std::shared_ptr<Expr> right = multiplication();
        expr = std::make_shared<Binary>(expr, oper, right);
    }

    return expr;
}

std::shared_ptr<Expr> Parser::multiplication()
{
    std::shared_ptr<Expr> expr = unary();

    while (match(TokenType::STAR, TokenType::SLASH))
    {
        Token oper = previous();
        std::shared_ptr<Expr> right = unary();
        expr = std::make_shared<Binary>(expr, oper, right);
    }

    return expr;
}

std::shared_ptr<Expr> Parser::unary()
{
    if (match(TokenType::BANG, TokenType::MINUS,
              TokenType::PLUS_PLUS, TokenType::MINUS_MINUS))
    {
        Token oper = previous();
        std::shared_ptr<Expr> right = unary();

        if (oper.type == TokenType::PLUS_PLUS || oper.type == TokenType::MINUS_MINUS)
        {
            if (auto var = std::dynamic_pointer_cast<Variable>(right))
            {
                return std::make_shared<Increment>(oper, var, true);
            }
            throw std::runtime_error("Increment/decrement can only be applied to variables");
        }

        return std::make_shared<Unary>(oper, right);
    }

    return call();
}

std::shared_ptr<Expr> Parser::call()
{
    std::shared_ptr<Expr> expr = basicPrimary();

    while (true)
    {
        if (match(TokenType::LEFT_PAREN))
        {
            expr = finishFunctionCall(expr);
        }
        else if (match(TokenType::LEFT_BRACKET))
        {
            expr = finishArrayAccess(expr);
        }
        else
        {
            break;
        }
    }

    return expr;
}

std::shared_ptr<Expr> Parser::finishFunctionCall(std::shared_ptr<Expr> callee)
{
    std::vector<std::shared_ptr<Expr>> arguments;

    if (!check(TokenType::RIGHT_PAREN))
    {
        do
        {
            if (arguments.size() >= 255)
            {
                throw std::runtime_error("Can't have more than 255 arguments.");
            }
            arguments.push_back(expression());
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments.");
    return std::make_shared<FunctionCall>(callee, arguments);
}

std::shared_ptr<Expr> Parser::finishArrayAccess(std::shared_ptr<Expr> array)
{
    std::shared_ptr<Expr> index = expression();
    consume(TokenType::RIGHT_BRACKET, "Expect ']' after index.");

    return std::make_shared<ArrayAccess>(array, index);
}

std::shared_ptr<Expr> Parser::arrayLiteral()
{
    std::vector<std::shared_ptr<Expr>> elements;

    if (!check(TokenType::RIGHT_BRACKET))
    {
        do
        {
            elements.push_back(expression());
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RIGHT_BRACKET, "Expect ']' after array elements.");
    return std::make_shared<ArrayLiteral>(elements);
}

std::shared_ptr<Expr> Parser::basicPrimary()
{
    if (match(TokenType::STRING) || match(TokenType::NUMBER))
    {
        return std::make_shared<Literal>(previous().literal);
    }

    if (match(TokenType::NIL)) {
        return std::make_shared<Literal>(nullptr);
    }

    if (match(TokenType::TRUE))
    {
        return std::make_shared<Literal>(true);
    }
    if (match(TokenType::FALSE))
    {
        return std::make_shared<Literal>(false);
    }

    if (match(TokenType::LEFT_PAREN))
    {
        std::shared_ptr<Expr> expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return std::make_shared<Grouping>(expr);
    }

    if (match(TokenType::LEFT_BRACKET))
    {
        return arrayLiteral();
    }

    if (match(TokenType::IDENTIFIER) || match(TokenType::TO_STRING) ||
        match(TokenType::INPUT) || match(TokenType::TO_NUMBER) || match(TokenType::CLEAR))
    {
        Token name = previous();
        auto variable = std::make_shared<Variable>(name);

        // Verifica se é chamada de função
        if (match(TokenType::LEFT_PAREN))
        {
            return finishFunctionCall(variable);
        }

        return variable;
    }

    if (match(TokenType::INCLUDE))
    {
        Token includeToken = previous();

        // Deve ser seguido por parênteses
        consume(TokenType::LEFT_PAREN, "Expect '(' after include");

        // O argumento deve ser uma string
        if (!check(TokenType::STRING))
        {
            throw std::runtime_error("include() expects a string literal");
        }

        std::shared_ptr<Expr> filename = std::make_shared<Literal>(peek().literal);
        advance(); // Consome a string

        consume(TokenType::RIGHT_PAREN, "Expect ')' after include filename");

        // Cria uma chamada de função include
        std::vector<std::shared_ptr<Expr>> args = {filename};
        auto includeVar = std::make_shared<Variable>(includeToken);
        return std::make_shared<FunctionCall>(includeVar, args);
    }

    if (match(TokenType::RETURN))
    {
        Token keyword = previous();
        std::shared_ptr<Expr> value = nullptr;
        if (!check(TokenType::SEMICOLON))
        {
            value = expression();
        }
        return std::make_shared<Return>(keyword, value);
    }

    // MELHOR MENSAGEM DE ERRO
    Token current = peek();
    throw std::runtime_error("Expect expression. Found: '" + current.lexeme + "' at line " + std::to_string(current.line));
}

bool Parser::isAtEnd()
{
    return peek().type == TokenType::MONNY_EOF;
}

Token Parser::advance()
{
    if (!isAtEnd())
        current++;
    return previous();
}

Token Parser::peek()
{
    if (current >= tokens.size())
    {
        return Token(TokenType::MONNY_EOF, "", std::any(), 0);
    }
    return tokens[current];
}

Token Parser::previous()
{
    if (current == 0)
    {
        return Token(TokenType::MONNY_EOF, "", std::any(), 0);
    }
    return tokens[current - 1];
}

bool Parser::check(TokenType type)
{
    return !isAtEnd() && peek().type == type;
}

template <class... T>
bool Parser::match(T... types)
{
    if ((check(types) || ...))
    {
        advance();
        return true;
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string &message)
{
    if (check(type))
        return advance();
    throw std::runtime_error(message);
}