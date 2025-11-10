#pragma once
#include <vector>
#include <memory>
#include <string>
#include <tokenizer/Token.hpp>

class Expr;
class Binary;
class Literal;
class Grouping;
class Variable;
class Assign;
class FunctionCall;
class Increment;
class Return;
class ArrayAccess;
class ArrayAssign;
class ArrayLiteral;

namespace Statements {
    class Stmt;
    class Print;
    class Var;
    class Expression;
    class IF;
    class Block;
    class While;
    class Clear;
    class For;
    class FunctionDef;
    class Const;
}

class Parser {
private:
    std::vector<Token> tokens;
    size_t current = 0;
    
    // Funções auxiliares
    bool isAtEnd();
    Token peek();
    Token advance();
    Token previous();
    bool check(TokenType type);
    
    template<class... T>
    bool match(T... types);
    
    Token consume(TokenType type, const std::string& message);
    
    // Expressões
    std::shared_ptr<Expr> expression();
    std::shared_ptr<Expr> equality();
    std::shared_ptr<Expr> comparison();
    std::shared_ptr<Expr> addition();
    std::shared_ptr<Expr> multiplication();
    std::shared_ptr<Expr> assignment();
    std::shared_ptr<Expr> logicalOr();
    std::shared_ptr<Expr> logicalAnd();
    std::shared_ptr<Expr> basicPrimary();
    std::shared_ptr<Expr> unary();
    std::shared_ptr<Expr> arrayLiteral();
    std::shared_ptr<Expr> call();
    std::shared_ptr<Expr> finishArrayAccess(std::shared_ptr<Expr> array);

    // Statements
    std::vector<std::shared_ptr<Statements::Stmt>> block();    
    std::shared_ptr<Statements::Stmt> statement();
    std::shared_ptr<Statements::Print> printStatement();
    std::shared_ptr<Statements::Var> varStatement();
    std::shared_ptr<Statements::Expression> expressionStatement();
    std::shared_ptr<Statements::IF> ifStatement();
    std::shared_ptr<Statements::While> whileStatement();
    std::shared_ptr<Statements::Stmt> incrementStatement();
    std::shared_ptr<Statements::Clear> clearStatement();
    std::shared_ptr<Statements::Stmt> forStatement();
    std::shared_ptr<Statements::FunctionDef> functionStatement();
    std::shared_ptr<Statements::Const> constStatement();
    
    // Funções
    std::shared_ptr<Expr> finishFunctionCall(std::shared_ptr<Expr> callee);

public:
    Parser(const std::vector<Token>& tokens);
    std::vector<std::shared_ptr<Statements::Stmt>> parse();
};