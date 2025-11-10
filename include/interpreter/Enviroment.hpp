#pragma once
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <any>
#include <memory>
#include <stdexcept>
#include <vector>

class Environment : public std::enable_shared_from_this<Environment>
{
private:
    std::vector<std::unordered_map<std::string, std::any>> scopes;
    std::unordered_set<std::string> constants;
    std::shared_ptr<Environment> parent;

public:
    Environment() : parent(nullptr)
    {
        // Inicia com escopo global
        enter_scope();
    }

    // Construtor com parent (para funções)
    Environment(std::shared_ptr<Environment> parent) : parent(parent)
    {
        // Inicia com escopo local
        enter_scope();
    }

    // Entra em um novo escopo (bloco)
    void enter_scope()
    {
        scopes.push_back({});
    }

    // Sai do escopo atual (variáveis morrem)
    void exit_scope()
    {
        if (scopes.size() > 1)
        { // Não remove o escopo global
            scopes.pop_back();
        }
    }

    void define(const std::string &name, std::any value, bool isConst = false)
    {
        if (scopes.back().count(name))
        {
            throw std::runtime_error("Variable '" + name + "' has already been defined in this scope");
        }
        scopes.back()[name] = value;
        if (isConst)
        {
            constants.insert(name);
        }
    }

    void assign(const std::string &name, std::any value)
    {
        // Verifica se é constante
        if (constants.count(name))
        {
            throw std::runtime_error("Cannot assign to constant '" + name + "'");
        }

        // Procura do escopo mais interno para o mais externo
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
        {
            if (it->count(name))
            {
                it->at(name) = value;
                return;
            }
        }

        // Se não encontrou nos escopos locais, procura no parent
        if (parent != nullptr)
        {
            parent->assign(name, value);
            return;
        }

        throw std::runtime_error("Undefined variable '" + name + "'.");
    }

    std::any get(const std::string &name)
    {
        // Procura do escopo mais interno para o mais externo
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
        {
            if (it->count(name))
            {
                return it->at(name);
            }
        }

        // Se não encontrou nos escopos locais, procura no parent
        if (parent != nullptr)
        {
            return parent->get(name);
        }

        throw std::runtime_error("Undefined variable '" + name + "'.");
    }

    // Método auxiliar para verificar se uma variável existe no escopo atual
    bool exists_in_current_scope(const std::string &name)
    {
        return scopes.back().count(name) > 0;
    }
};