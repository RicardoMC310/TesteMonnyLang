#include <iostream>
#include <Monny.hpp>

int main(int argc, char **argv)
{

    if (argc > 2)
    {
        std::cerr << "Usage: " << argv[0] << " file.mn.\n";
    }

    if (argc == 2)
    {
        Monny::runScriptFile(argv[1]);
    }
    else
    {
        Monny::runREPL();
    }

    return EXIT_SUCCESS;
}