#include <iostream>
#include "assembler.h"
#include "preprocessor.h"
#include "utils.h"



int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " -p input.asm | -o input.pre" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    std::string inputFile = argv[2];

    try
    {
        Utils utils;
        if (mode == "-p")
        {
            Preprocessor preprocessor;
            std::string preprocessedFile = utils.replaceExtension(inputFile, ".pre");
            preprocessor.preprocess(inputFile, preprocessedFile);
        }
        else if (mode == "-o")
        {
            Assembler assembler;
            std::string preprocessedFile = utils.replaceExtension(inputFile, ".obj");
            assembler.assemble(inputFile, preprocessedFile);
        }
        else
        {
            std::cerr << "Unknown mode: " << mode << std::endl;
            return 1;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
