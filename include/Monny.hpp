#pragma once

#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>

class Monny {
private:
    static void run(const std::string&);
public:
    static void runScriptFile(const std::string&);
    static void runREPL();
};