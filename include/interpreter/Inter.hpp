#pragma once
#include <vector>
#include <memory>
#include <any>
#include <iostream>
#include <parser/Expr.hpp>
#include <parser/Stmt.hpp>
#include <interpreter/Enviroment.hpp>

class Interpreter
{
private:
    std::shared_ptr<Environment> environment = std::make_shared<Environment>();
    std::any result;

    // Estrutura simples para funções (sem classe nova)
    struct FunctionData
    {
        std::shared_ptr<Statements::FunctionDef> declaration;
    };

    // Usa uma exceção especial para retorno
    class ReturnException : public std::exception
    {
    public:
        std::any value;
        ReturnException(std::any v) : value(v) {}
        const char *what() const noexcept override { return "Return exception"; }
    };

public:
    Interpreter() = default;

    // Interface pública principal
    void interpret(const std::vector<std::shared_ptr<Statements::Stmt>> &statements);

    // Execução de statements
    void execute(std::shared_ptr<Statements::Stmt> stmt);
    void executePrint(std::shared_ptr<Statements::Print> stmt);
    void executeExpression(std::shared_ptr<Statements::Expression> stmt);
    void executeIf(std::shared_ptr<Statements::IF> stmt);
    void executeVar(std::shared_ptr<Statements::Var> stmt);
    void executeBlock(std::shared_ptr<Statements::Block> stmt);
    void executeWhile(std::shared_ptr<Statements::While> stmt);
    void executeClear(std::shared_ptr<Statements::Clear> stmt);
    void executeFunctionDef(std::shared_ptr<Statements::FunctionDef> stmt);
    void executeConst(std::shared_ptr<Statements::Const> stmt);

    // Avaliação de expressões
    std::any evaluate(std::shared_ptr<Expr> expr);
    std::any evaluateBinary(std::shared_ptr<Binary> expr);
    std::any evaluateLiteral(std::shared_ptr<Literal> expr);
    std::any evaluateGrouping(std::shared_ptr<Grouping> expr);
    std::any evaluateVariable(std::shared_ptr<Variable> expr);
    std::any evaluateAssign(std::shared_ptr<Assign> expr);
    std::any evaluateFunctionCall(std::shared_ptr<FunctionCall> expr);
    std::any evaluateIncrement(std::shared_ptr<Increment> expr);
    std::any evaluateUnary(std::shared_ptr<Unary> expr);
    std::any evaluateLogical(std::shared_ptr<Logical> expr);
    std::any evaluateReturn(std::shared_ptr<Return> expr);
    std::any evaluateArrayLiteral(std::shared_ptr<ArrayLiteral> expr);
    std::any evaluateArrayAccess(std::shared_ptr<ArrayAccess> expr);
    std::any evaluateArrayAssign(std::shared_ptr<ArrayAssign> expr);

    std::any callUserFunction(const std::string &name,
                              const std::vector<std::any> &arguments,
                              std::shared_ptr<Statements::FunctionDef> funcDef);

    std::any executeFile(const std::string &filename);

private:
    // Funções auxiliares
    bool isTruthy(std::any value);
    bool isEqual(std::any a, std::any b);
    void checkNumberOperand(const Token &oper, std::any operand);
    void checkNumberOperands(const Token &oper, std::any left, std::any right);
    std::string stringify(std::any value);
};