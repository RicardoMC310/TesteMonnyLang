// ArrayObject.hpp (crie este arquivo)
#pragma once
#include <vector>
#include <any>
#include <string>

class ArrayObject
{
public:
    std::vector<std::any> elements;

    ArrayObject(std::vector<std::any> elements) : elements(elements) {}

    std::string toString() const
    {
        std::string result = "[";
        for (size_t i = 0; i < elements.size(); i++)
        {
            if (i > 0)
                result += ", ";

            // Converter cada elemento para string
            if (elements[i].type() == typeid(std::string))
            {
                result += "\"" + std::any_cast<std::string>(elements[i]) + "\"";
            }
            else if (elements[i].type() == typeid(bool))
            {
                result += std::any_cast<bool>(elements[i]) ? "true" : "false";
            }
            else if (elements[i].type() == typeid(double))
            {
                std::string num = std::to_string(std::any_cast<double>(elements[i]));
                // Remover zeros desnecessários
                num.erase(num.find_last_not_of('0') + 1, std::string::npos);
                num.erase(num.find_last_not_of('.') + 1, std::string::npos);
                result += num;
            }
            else if (elements[i].type() == typeid(nullptr))
            {
                result += "nil";
            }
            else
            {
                result += "?";
            }
        }
        result += "]";
        return result;
    }
};