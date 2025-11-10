#include <Monny.hpp>
#include <string>
#include <utils/Systems.hpp>
#include <tokenizer/Scanner.hpp>
#include <parser/Parser.hpp>
#include <parser/Expr.hpp>
#include <parser/Stmt.hpp>
#include <interpreter/Inter.hpp>

namespace fs = std::filesystem;

void Monny::runScriptFile(const std::string &path)
{
	if (!fs::exists(path))
	{
		std::cout << "[ERROR]: file not found.\n";
		std::exit(66);
	}

	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file)
	{
		std::cout << "[ERROR]: permission reading file.\n";
		std::exit(66);
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);

	if (!file.read((char *)buffer.data(), size))
	{
		std::cout << "[ERROR]: file is not complete.\n";
	}

	std::string source(buffer.begin(), buffer.end());
	run(source);
}

void Monny::runREPL()
{
	System::clear();
	std::cout << "monny> ";
	std::string line;

	for (;;)
	{
		if (!std::getline(std::cin, line) || line == "exit")
		{
			break;
		}
		if (line == "clear")
		{
			System::clear();
			std::cout << "\nmonny> ";
			continue;
		}
		run(line);
		line = "";
		std::cout << "\nmonny> ";
	}
}

void Monny::run(const std::string &source)
{
	Scanner scanner(source);
	std::vector<Token> tokens = scanner.scanTokens();

	Parser parser(tokens);
	auto statements = parser.parse();

	Interpreter inter;
	inter.interpret(statements);
}