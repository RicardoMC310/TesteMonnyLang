#include <napi.h>
#include <include/parser/Parser.hpp>
#include <include/tokenizer/Scanner.hpp>

// Exemplo simples: função que retorna tokens
Napi::Value Tokenize(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Código fonte esperado como string").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string source = info[0].As<Napi::String>();
    Monny::Tokenizer tokenizer(source);
    auto tokens = tokenizer.scanTokens();

    Napi::Array result = Napi::Array::New(env, tokens.size());
    for (size_t i = 0; i < tokens.size(); ++i) {
        Napi::Object tokenObj = Napi::Object::New(env);
        tokenObj.Set("type", tokens[i].type);
        tokenObj.Set("lexeme", tokens[i].lexeme);
        tokenObj.Set("line", tokens[i].line);
        result[i] = tokenObj;
    }

    return result;
}

// Outro exemplo: interpretar o código
Napi::Value RunCode(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    std::string source = info[0].As<Napi::String>();
    Monny::Interpreter interpreter;
    interpreter.run(source);
    return env.Null();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("tokenize", Napi::Function::New(env, Tokenize));
    exports.Set("run", Napi::Function::New(env, RunCode));
    return exports;
}

NODE_API_MODULE(monny_core, Init)
