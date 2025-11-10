#pragma once

#include <any>
#include <memory>
#include <vector>

#include <tokenizer/Token.hpp>

class Expr
{
public:
    virtual ~Expr() = default;
};

class Literal : public Expr
{
public:
    std::any value;

    Literal(std::any value) : value(value) {}
};

class FunctionCall : public Expr {
public:
    std::shared_ptr<Expr> callee;  // Mude de Token para shared_ptr<Expr>
    std::vector<std::shared_ptr<Expr>> arguments;
    
    FunctionCall(std::shared_ptr<Expr> callee, std::vector<std::shared_ptr<Expr>> arguments)
        : callee(callee), arguments(arguments) {}
};

class Variable : public Expr
{
public:
    Token name;

    Variable(Token name) : name(name) {}
};

class Binary : public Expr
{
public:
    std::shared_ptr<Expr> left;
    Token oper;
    std::shared_ptr<Expr> right;

    Binary(std::shared_ptr<Expr> left, Token oper, std::shared_ptr<Expr> right) : left(left), oper(oper), right(right) {}
};

class Grouping : public Expr
{
public:
    std::shared_ptr<Expr> expression;

    Grouping(std::shared_ptr<Expr> expression) : expression(expression) {}
};

class Assign : public Expr
{
public:
    Token name;
    std::shared_ptr<Expr> value;

    Assign(Token name, std::shared_ptr<Expr> value) : name(name), value(value) {}
};

class Increment : public Expr
{
public:
    Token oper;
    std::shared_ptr<Expr> operand;
    bool isPrefix;

    Increment(Token oper, std::shared_ptr<Expr> operand, bool isPrefix)
        : oper(oper), operand(operand), isPrefix(isPrefix) {}
};

class Unary : public Expr
{
public:
    Token oper;
    std::shared_ptr<Expr> right;

    Unary(Token oper, std::shared_ptr<Expr> right)
        : oper(oper), right(right) {}
};

class Logical : public Expr
{
public:
    std::shared_ptr<Expr> left;
    Token oper;
    std::shared_ptr<Expr> right;

    Logical(std::shared_ptr<Expr> left, Token oper, std::shared_ptr<Expr> right)
        : left(left), oper(oper), right(right) {}
};

class Return : public Expr
{
public:
    Token keyword;
    std::shared_ptr<Expr> value;

    Return(Token keyword, std::shared_ptr<Expr> value)
        : keyword(keyword), value(value) {}
};

// Array literal: [1, "hello", true]
class ArrayLiteral : public Expr {
public:
    std::vector<std::shared_ptr<Expr>> elements;
    
    ArrayLiteral(std::vector<std::shared_ptr<Expr>> elements)
        : elements(elements) {}
};

// Acesso a array: arr[0]
class ArrayAccess : public Expr {
public:
    std::shared_ptr<Expr> array;
    std::shared_ptr<Expr> index;
    
    ArrayAccess(std::shared_ptr<Expr> array, std::shared_ptr<Expr> index)
        : array(array), index(index) {}
};

// Atribuição a array: arr[0] = 5
class ArrayAssign : public Expr {
public:
    std::shared_ptr<Expr> array;
    std::shared_ptr<Expr> index;
    std::shared_ptr<Expr> value;
    
    ArrayAssign(std::shared_ptr<Expr> array, std::shared_ptr<Expr> index, std::shared_ptr<Expr> value)
        : array(array), index(index), value(value) {}
};