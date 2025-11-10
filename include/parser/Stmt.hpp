#pragma once

#include <vector>
#include <memory>

#include <parser/Expr.hpp>

namespace Statements
{
    class Stmt
    {
    public:
        virtual ~Stmt() = default;
    };

    class Print : public Stmt
    {
    public:
        std::vector<std::shared_ptr<Expr>> expressions;

        Print(std::vector<std::shared_ptr<Expr>> expressions) : expressions(std::move(expressions)) {}
    };

    class Var : public Stmt
    {
    public:
        Token name;
        std::shared_ptr<Expr> initializer;

        Var(Token name, std::shared_ptr<Expr> initializer) : name(name), initializer(initializer) {}
    };

    class Expression : public Stmt
    {
    public:
        std::shared_ptr<Expr> expression;

        Expression(std::shared_ptr<Expr> expression) : expression(expression) {}
    };

    class IF : public Stmt
    {
    public:
        std::shared_ptr<Expr> condition;
        std::shared_ptr<Stmt> thenBranch;
        std::shared_ptr<Stmt> elseBranch;

        IF(std::shared_ptr<Expr> condition,
           std::shared_ptr<Stmt> thenBranch,
           std::shared_ptr<Stmt> elseBranch)
            : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}
    };

    class Block : public Stmt
    {
    public:
        std::vector<std::shared_ptr<Stmt>> statements;

        Block(std::vector<std::shared_ptr<Stmt>> statements)
            : statements(std::move(statements)) {}
    };

    class While : public Stmt
    {
    public:
        std::shared_ptr<Expr> condition;
        std::shared_ptr<Stmt> body;

        While(std::shared_ptr<Expr> condition,
              std::shared_ptr<Stmt> body)
            : condition(condition), body(body) {}
    };

    class Clear : public Stmt
    {
    public:
        Clear() = default;
    };

    class For : public Stmt
    {
    public:
        std::shared_ptr<Stmt> initializer;
        std::shared_ptr<Expr> condition;
        std::shared_ptr<Expr> increment;
        std::shared_ptr<Stmt> body;

        For(std::shared_ptr<Stmt> initializer,
            std::shared_ptr<Expr> condition,
            std::shared_ptr<Expr> increment,
            std::shared_ptr<Stmt> body)
            : initializer(initializer), condition(condition),
              increment(increment), body(body) {}
    };

    class FunctionDef : public Stmt
    {
    public:
        Token name;
        std::vector<Token> params;
        std::shared_ptr<Block> body;

        FunctionDef(Token name, std::vector<Token> params, std::shared_ptr<Block> body)
            : name(name), params(params), body(body) {}
    };

    class Const : public Stmt {
public:
    Token name;
    std::shared_ptr<Expr> initializer;
    
    Const(Token name, std::shared_ptr<Expr> initializer)
        : name(name), initializer(initializer) {}
};
}
