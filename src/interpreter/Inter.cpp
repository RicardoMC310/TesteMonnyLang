#include <interpreter/Inter.hpp>
#include <interpreter/ArrayObject.hpp>
#include <tokenizer/Scanner.hpp>
#include <parser/Parser.hpp>
#include <utils/Systems.hpp>
#include <sstream>
#include <iostream>
#include <limits>
#include <fstream>

// ========== INTERFACE PÚBLICA ==========

void Interpreter::interpret(const std::vector<std::shared_ptr<Statements::Stmt>> &statements)
{
    try
    {
        for (const auto &statement : statements)
        {
            execute(statement);
        }
    }
    catch (const std::runtime_error &error)
    {
        std::cerr << "Runtime error: " << error.what() << std::endl;
    }
}

void Interpreter::execute(std::shared_ptr<Statements::Stmt> stmt)
{
    if (auto printStmt = std::dynamic_pointer_cast<Statements::Print>(stmt))
    {
        executePrint(printStmt);
    }
    else if (auto exprStmt = std::dynamic_pointer_cast<Statements::Expression>(stmt))
    {
        executeExpression(exprStmt);
    }
    else if (auto ifStmt = std::dynamic_pointer_cast<Statements::IF>(stmt))
    {
        executeIf(ifStmt);
    }
    else if (auto varStmt = std::dynamic_pointer_cast<Statements::Var>(stmt))
    {
        executeVar(varStmt);
    }
    else if (auto blockStmt = std::dynamic_pointer_cast<Statements::Block>(stmt))
    {
        executeBlock(blockStmt);
    }
    else if (auto whileStmt = std::dynamic_pointer_cast<Statements::While>(stmt))
    {
        executeWhile(whileStmt);
    }
    else if (auto clearStmt = std::dynamic_pointer_cast<Statements::Clear>(stmt))
    {
        executeClear(clearStmt);
    }
    else if (auto funcDef = std::dynamic_pointer_cast<Statements::FunctionDef>(stmt))
    {
        executeFunctionDef(funcDef);
    }
    else if (auto constStmt = std::dynamic_pointer_cast<Statements::Const>(stmt))
    {
        executeConst(constStmt);
    }
}

// ========== IMPLEMENTAÇÃO DOS STATEMENTS ==========

void Interpreter::executeConst(std::shared_ptr<Statements::Const> stmt)
{
    // CONST sempre tem initializer (obrigatório pelo parser)
    std::any value = evaluate(stmt->initializer);

    // Define como constante (terceiro parâmetro = true)
    environment->define(stmt->name.lexeme, value, true);
}

void Interpreter::executeFunctionDef(std::shared_ptr<Statements::FunctionDef> stmt)
{
    // Armazena a definição da função diretamente no environment
    FunctionObject funcData{stmt, environment};
    environment->define(stmt->name.lexeme, funcData, false);
}

void Interpreter::executeClear(std::shared_ptr<Statements::Clear> stmt)
{
    System::clear();
}

void Interpreter::executePrint(std::shared_ptr<Statements::Print> stmt)
{
    for (size_t i = 0; i < stmt->expressions.size(); i++)
    {
        std::any value = evaluate(stmt->expressions[i]);

        // Se for string, processa caracteres de escape
        if (value.type() == typeid(std::string))
        {
            std::string str = std::any_cast<std::string>(value);
            std::cout << processEscapeSequences(str);
        }
        else
        {
            std::cout << stringify(value);
        }
    }
}

void Interpreter::executeExpression(std::shared_ptr<Statements::Expression> stmt)
{
    evaluate(stmt->expression);
}

void Interpreter::executeIf(std::shared_ptr<Statements::IF> stmt)
{
    if (isTruthy(evaluate(stmt->condition)))
    {
        execute(stmt->thenBranch);
    }
    else if (stmt->elseBranch != nullptr)
    {
        execute(stmt->elseBranch);
    }
}

void Interpreter::executeVar(std::shared_ptr<Statements::Var> stmt)
{
    std::any value = nullptr;
    if (stmt->initializer != nullptr)
    {
        value = evaluate(stmt->initializer);
    }
    environment->define(stmt->name.lexeme, value);
}

void Interpreter::executeWhile(std::shared_ptr<Statements::While> stmt)
{
    while (isTruthy(evaluate(stmt->condition)))
    {
        execute(stmt->body);
    }
}

void Interpreter::executeBlock(std::shared_ptr<Statements::Block> stmt)
{
    environment->enter_scope();
    try
    {
        for (const auto &statement : stmt->statements)
        {
            execute(statement);
        }
    }
    catch (const ReturnException &)
    {
        // Re-lança a exceção de return para que seja capturada pelo callUserFunction
        environment->exit_scope();
        throw;
    }
    catch (const std::exception &e)
    {
        environment->exit_scope();
    }

    environment->exit_scope();
}

// ========== IMPLEMENTAÇÃO DAS EXPRESSÕES ==========

std::any Interpreter::evaluate(std::shared_ptr<Expr> expr)
{
    if (auto binary = std::dynamic_pointer_cast<Binary>(expr))
    {
        return evaluateBinary(binary);
    }
    else if (auto literal = std::dynamic_pointer_cast<Literal>(expr))
    {
        return evaluateLiteral(literal);
    }
    else if (auto grouping = std::dynamic_pointer_cast<Grouping>(expr))
    {
        return evaluateGrouping(grouping);
    }
    else if (auto variable = std::dynamic_pointer_cast<Variable>(expr))
    {
        return evaluateVariable(variable);
    }
    else if (auto assign = std::dynamic_pointer_cast<Assign>(expr))
    {
        return evaluateAssign(assign);
    }
    else if (auto funcCall = std::dynamic_pointer_cast<FunctionCall>(expr))
    {
        return evaluateFunctionCall(funcCall);
    }
    else if (auto incre = std::dynamic_pointer_cast<Increment>(expr))
    {
        return evaluateIncrement(incre);
    }
    else if (auto unary = std::dynamic_pointer_cast<Unary>(expr))
    {
        return evaluateUnary(unary); // ← NOVO
    }
    else if (auto logical = std::dynamic_pointer_cast<Logical>(expr))
    {
        return evaluateLogical(logical); // ← NOVO
    }
    else if (auto returnExpr = std::dynamic_pointer_cast<Return>(expr))
    {
        return evaluateReturn(returnExpr);
    }
    else if (auto arrayLit = std::dynamic_pointer_cast<ArrayLiteral>(expr))
    {
        return evaluateArrayLiteral(arrayLit);
    }
    else if (auto arrayAccess = std::dynamic_pointer_cast<ArrayAccess>(expr))
    {
        return evaluateArrayAccess(arrayAccess);
    }
    else if (auto arrayAssign = std::dynamic_pointer_cast<ArrayAssign>(expr))
    {
        return evaluateArrayAssign(arrayAssign);
    }

    throw std::runtime_error("Unknown expression type");
}

std::any Interpreter::evaluateReturn(std::shared_ptr<Return> expr)
{
    std::any value = nullptr;
    if (expr->value != nullptr)
    {
        value = evaluate(expr->value);
    }

    // CORREÇÃO: Lança a exceção em vez de retornar
    throw ReturnException(value);
}

std::any Interpreter::evaluateLogical(std::shared_ptr<Logical> expr)
{
    std::any left = evaluate(expr->left);

    // Short-circuit evaluation
    if (expr->oper.type == TokenType::OR)
    {
        if (isTruthy(left))
            return left; // Se left é true, retorna true
    }
    else
    { // AND
        if (!isTruthy(left))
            return left; // Se left é false, retorna false
    }

    return evaluate(expr->right);
}

std::any Interpreter::evaluateUnary(std::shared_ptr<Unary> expr)
{
    std::any right = evaluate(expr->right);

    switch (expr->oper.type)
    {
    case TokenType::MINUS:
        checkNumberOperand(expr->oper, right);
        return -std::any_cast<double>(right);

    case TokenType::BANG:
        return !isTruthy(right);

    default:
        throw std::runtime_error("Unknown unary operator");
    }
}

std::any Interpreter::evaluateArrayLiteral(std::shared_ptr<ArrayLiteral> expr)
{
    std::vector<std::any> elements;
    for (const auto &element : expr->elements)
    {
        elements.push_back(evaluate(element));
    }
    return std::make_shared<ArrayObject>(elements);
}

std::any Interpreter::evaluateArrayAccess(std::shared_ptr<ArrayAccess> expr)
{
    std::any arrayAny = evaluate(expr->array);
    std::any indexAny = evaluate(expr->index);

    if (auto array = std::any_cast<std::shared_ptr<ArrayObject>>(&arrayAny))
    {
        if (indexAny.type() == typeid(double))
        {
            int index = static_cast<int>(std::any_cast<double>(indexAny));

            if (index >= 0 && index < (*array)->elements.size())
            {
                return (*array)->elements[index];
            }
            throw std::runtime_error("Array index out of bounds");
        }
        throw std::runtime_error("Array index must be a number");
    }
    throw std::runtime_error("Expected array");
}

std::any Interpreter::evaluateArrayAssign(std::shared_ptr<ArrayAssign> expr)
{
    std::any arrayAny = evaluate(expr->array);
    std::any indexAny = evaluate(expr->index);
    std::any value = evaluate(expr->value);

    if (auto array = std::any_cast<std::shared_ptr<ArrayObject>>(&arrayAny))
    {
        if (indexAny.type() == typeid(double))
        {
            int index = static_cast<int>(std::any_cast<double>(indexAny));

            if (index >= 0 && index < (*array)->elements.size())
            {
                (*array)->elements[index] = value;
                return value;
            }
            throw std::runtime_error("Array index out of bounds");
        }
        throw std::runtime_error("Array index must be a number");
    }
    throw std::runtime_error("Expected array");
}

std::any Interpreter::evaluateIncrement(std::shared_ptr<Increment> expr)
{
    auto varExpr = std::dynamic_pointer_cast<Variable>(expr->operand);
    if (!varExpr)
    {
        throw std::runtime_error("Increment/decrement can only be applied to variables");
    }

    std::string varName = varExpr->name.lexeme;
    std::any currentValue = environment->get(varName);

    if (currentValue.type() != typeid(double))
    {
        throw std::runtime_error("Increment/decrement can only be applied to numbers");
    }

    double value = std::any_cast<double>(currentValue);
    double change = (expr->oper.type == TokenType::PLUS_PLUS) ? 1.0 : -1.0;
    double newValue = value + change;

    environment->assign(varName, newValue);

    if (expr->isPrefix)
    {
        return newValue;
    }
    else
    {
        return value;
    }
}

std::any Interpreter::evaluateBinary(std::shared_ptr<Binary> expr)
{
    std::any left = evaluate(expr->left);
    std::any right = evaluate(expr->right);

    switch (expr->oper.type)
    {
    case TokenType::GREATER:
        checkNumberOperands(expr->oper, left, right);
        return std::any_cast<double>(left) > std::any_cast<double>(right);

    case TokenType::GREATER_EQUAL:
        checkNumberOperands(expr->oper, left, right);
        return std::any_cast<double>(left) >= std::any_cast<double>(right);

    case TokenType::LESS:
        checkNumberOperands(expr->oper, left, right);
        return std::any_cast<double>(left) < std::any_cast<double>(right);

    case TokenType::LESS_EQUAL:
        checkNumberOperands(expr->oper, left, right);
        return std::any_cast<double>(left) <= std::any_cast<double>(right);

    case TokenType::EQUAL_EQUAL:
        return isEqual(left, right);

    case TokenType::BANG_EQUAL:
        return !isEqual(left, right);

    case TokenType::MINUS:
        checkNumberOperands(expr->oper, left, right);
        return std::any_cast<double>(left) - std::any_cast<double>(right);

    case TokenType::PLUS:
        if (left.type() == typeid(double) && right.type() == typeid(double))
        {
            return std::any_cast<double>(left) + std::any_cast<double>(right);
        }
        if (left.type() == typeid(std::string) && right.type() == typeid(std::string))
        {
            return std::any_cast<std::string>(left) + std::any_cast<std::string>(right);
        }
        throw std::runtime_error("Operands must be two numbers or two strings.");

    case TokenType::SLASH:
        checkNumberOperands(expr->oper, left, right);
        return std::any_cast<double>(left) / std::any_cast<double>(right);

    case TokenType::STAR:
        checkNumberOperands(expr->oper, left, right);
        return std::any_cast<double>(left) * std::any_cast<double>(right);
    }

    return nullptr;
}

std::any Interpreter::evaluateLiteral(std::shared_ptr<Literal> expr)
{
    return expr->value;
}

std::any Interpreter::evaluateGrouping(std::shared_ptr<Grouping> expr)
{
    return evaluate(expr->expression);
}

std::any Interpreter::evaluateVariable(std::shared_ptr<Variable> expr)
{
    return environment->get(expr->name.lexeme);
}

std::any Interpreter::evaluateAssign(std::shared_ptr<Assign> expr)
{
    std::any value = evaluate(expr->value);
    environment->assign(expr->name.lexeme, value);
    return value;
}

std::any Interpreter::evaluateFunctionCall(std::shared_ptr<FunctionCall> expr)
{
    // Para funções built-in, precisamos extrair o nome do callee
    std::string functionName;

    // Se o callee for um Variable, extraímos o nome do token
    if (auto var = std::dynamic_pointer_cast<Variable>(expr->callee))
    {
        functionName = var->name.lexeme;
    }
    else
    {
        // Para outros tipos de callee, avaliamos e tentamos converter
        std::any calleeValue = evaluate(expr->callee);
        // Lógica para outros tipos de callee...
        throw std::runtime_error("Complex function calls not yet supported");
    }

    // Agora usa functionName em vez de expr->name.lexeme
    if (functionName == "input")
    {
        std::string prompt;
        if (expr->arguments.size() != 1)
        {
            throw std::runtime_error("input() expects exactly 1 argument");
        }

        std::any promptVal = evaluate(expr->arguments[0]);
        prompt = stringify(promptVal);

        std::cout << prompt;

        if (std::cin.peek() == '\n')
        {
            std::cin.ignore();
        }

        std::string input;
        std::getline(std::cin, input);

        input.erase(0, input.find_first_not_of(" \t\n\r\f\v"));
        input.erase(input.find_last_not_of(" \t\n\r\f\v") + 1);

        return input;
    }
    else if (functionName == "to_string")
    {
        if (expr->arguments.size() != 1)
        {
            throw std::runtime_error("to_string() expects exactly 1 argument");
        }
        return stringify(evaluate(expr->arguments[0]));
    }
    else if (functionName == "to_number")
    {
        if (expr->arguments.size() != 1)
        {
            throw std::runtime_error("to_number() expects exactly 1 argument");
        }
        auto value = evaluate(expr->arguments[0]);
        if (value.type() == typeid(std::string))
        {
            try
            {
                return std::stod(std::any_cast<std::string>(value));
            }
            catch (...)
            {
                throw std::runtime_error("Cannot convert string to number");
            }
        }
        return value;
    }
    else if (functionName == "len")
    {
        if (expr->arguments.size() != 1)
        {
            throw std::runtime_error("len() expects exactly 1 argument");
        }
        std::any arg = evaluate(expr->arguments[0]);

        if (auto array = std::any_cast<std::shared_ptr<ArrayObject>>(&arg))
        {
            return static_cast<double>((*array)->elements.size());
        }
        if (arg.type() == typeid(std::string))
        {
            return static_cast<double>(std::any_cast<std::string>(arg).length());
        }
        throw std::runtime_error("len() expects array or string");
    }
    else if (functionName == "push")
    {
        if (expr->arguments.size() != 2)
        {
            throw std::runtime_error("push() expects exactly 2 arguments");
        }
        std::any arrayAny = evaluate(expr->arguments[0]);
        std::any value = evaluate(expr->arguments[1]);

        if (auto array = std::any_cast<std::shared_ptr<ArrayObject>>(&arrayAny))
        {
            (*array)->elements.push_back(value);
            return value;
        }
        throw std::runtime_error("push() expects array as first argument");
    }
    else if (functionName == "pop")
    {
        if (expr->arguments.size() != 1)
        {
            throw std::runtime_error("pop() expects exactly 1 argument");
        }
        std::any arrayAny = evaluate(expr->arguments[0]);

        if (auto array = std::any_cast<std::shared_ptr<ArrayObject>>(&arrayAny))
        {
            if ((*array)->elements.empty())
            {
                throw std::runtime_error("Cannot pop from empty array");
            }
            auto last = (*array)->elements.back();
            (*array)->elements.pop_back();
            return last;
        }
        throw std::runtime_error("pop() expects array");
    }
    if (functionName == "include")
    {
        if (expr->arguments.size() != 1)
        {
            throw std::runtime_error("include() expects exactly 1 argument");
        }

        std::any filenameAny = evaluate(expr->arguments[0]);
        if (filenameAny.type() != typeid(std::string))
        {
            throw std::runtime_error("include() expects a string filename");
        }

        std::string filename = std::any_cast<std::string>(filenameAny);
        return executeFile(filename);
    }

    try
    {
        std::any funcAny = environment->get(functionName);

        if (funcAny.type() == typeid(FunctionObject))
        {
            FunctionObject funcData = std::any_cast<FunctionObject>(funcAny);
            auto funcDef = funcData.declaration;

            // Avalia argumentos
            std::vector<std::any> arguments;
            for (const auto &arg : expr->arguments)
            {
                arguments.push_back(evaluate(arg));
            }

            // Verifica número de parâmetros
            if (arguments.size() != funcDef->params.size())
            {
                throw std::runtime_error("Expected " +
                                         std::to_string(funcDef->params.size()) +
                                         " arguments but got " +
                                         std::to_string(arguments.size()));
            }

            return callUserFunction(functionName, arguments, funcDef);
        }
    }
    catch (const std::runtime_error &e)
    {
        // Se não encontrou a variável, continua para ver se é built-in
    }

    throw std::runtime_error("Unknown function: " + functionName);
}

// ========== FUNÇÕES AUXILIARES ==========

bool Interpreter::isTruthy(std::any value)
{
    if (value.type() == typeid(bool))
    {
        return std::any_cast<bool>(value);
    }
    if (value.type() == typeid(nullptr))
    {
        return false;
    }
    if (value.type() == typeid(nullptr))
    {
        return false;
    }
    return true;
}

bool Interpreter::isEqual(std::any a, std::any b)
{
    if (a.type() != b.type())
    {
        // nil é igual apenas a nil
        if (a.type() == typeid(nullptr) && b.type() == typeid(nullptr))
        {
            return true;
        }
        return false;
    }

    if (a.type() == typeid(nullptr))
    {
        return true; // nil == nil
    }
    if (a.type() == typeid(bool))
    {
        return std::any_cast<bool>(a) == std::any_cast<bool>(b);
    }
    if (a.type() == typeid(double))
    {
        return std::any_cast<double>(a) == std::any_cast<double>(b);
    }
    if (a.type() == typeid(std::string))
    {
        return std::any_cast<std::string>(a) == std::any_cast<std::string>(b);
    }

    return false;
}

void Interpreter::checkNumberOperand(const Token &oper, std::any operand)
{
    if (operand.type() == typeid(double))
        return;
    throw std::runtime_error("Operand must be a number.");
}

void Interpreter::checkNumberOperands(const Token &oper, std::any left, std::any right)
{
    if (left.type() == typeid(double) && right.type() == typeid(double))
        return;
    throw std::runtime_error("Operands must be numbers.");
}

std::string Interpreter::stringify(std::any value)
{
    if (value.type() == typeid(nullptr))
        return "nil";
    if (value.type() == typeid(bool))
        return std::any_cast<bool>(value) ? "true" : "false";
    if (value.type() == typeid(double))
    {
        std::string text = std::to_string(std::any_cast<double>(value));
        text.erase(text.find_last_not_of('0') + 1, std::string::npos);
        text.erase(text.find_last_not_of('.') + 1, std::string::npos);
        return text;
    }
    if (value.type() == typeid(std::string))
        return std::any_cast<std::string>(value);
    if (value.type() == typeid(FunctionObject))
        return "<function>";
    if (value.type() == typeid(std::shared_ptr<ArrayObject>))
    {
        auto array = std::any_cast<std::shared_ptr<ArrayObject>>(value);
        return array->toString();
    }

    return "unknown";
}

std::any Interpreter::callUserFunction(const std::string &name,
                                       const std::vector<std::any> &arguments,
                                       std::shared_ptr<Statements::FunctionDef> funcDef)
{
    // Salva environment atual
    auto previousEnv = environment;

    // Cria novo environment para a função (com parent para acessar funções built-in)
    environment = std::make_shared<Environment>(previousEnv);

    // Define parâmetros
    for (size_t i = 0; i < funcDef->params.size(); i++)
    {
        environment->define(funcDef->params[i].lexeme, arguments[i]);
    }

    // Executa o corpo da função
    std::any result = nullptr;
    try
    {
        execute(funcDef->body);
    }
    catch (const ReturnException &ret)
    {
        result = ret.value;
    }
    catch (...)
    {
        environment = previousEnv;
        throw;
    }

    // Restaura environment
    environment = previousEnv;
    return result;
}

std::any Interpreter::executeFile(const std::string &filename)
{
    // Lê o conteúdo do arquivo
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    file.close();

    // Tokeniza e interpreta
    Scanner scanner(source);
    auto tokens = scanner.scanTokens();

    Parser parser(tokens);
    auto statements = parser.parse();

    // Executa as statements no mesmo environment
    for (const auto &stmt : statements)
    {
        execute(stmt);
    }

    return nullptr; // include não retorna valor
}

std::string Interpreter::processEscapeSequences(const std::string& str)
{
    std::string result;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            switch (str[i + 1]) {
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                case 'r': result += '\r'; break;
                case '\\': result += '\\'; break;
                case '"': result += '"'; break;
                default: result += str[i + 1]; break;
            }
            i++; // Pula o próximo caractere
        } else {
            result += str[i];
        }
    }
    return result;
}