#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <sstream>

void Linker::link(const std::string &inputFile1, const std::string &inputFile2, const std::string &outputFile)
{
    std::ifstream input1(inputFile1);
    std::ifstream input2(inputFile2);
    std::ofstream output(outputFile);

    std::unordered_map<std::string, int> globalDefinitionTable;
    std::unordered_map<std::string, std::vector<int>> usageTable1;
    std::unordered_map<std::string, std::vector<int>> usageTable2;

    // Lê as tabelas de definições e uso do primeiro módulo
    std::string line;
    bool readDefinitions = false;
    while (std::getline(input1, line))
    {
        if (line == "DEFINITION TABLE:")
        {
            readDefinitions = true;
            continue;
        }
        if (line == "USAGE TABLE:")
        {
            readDefinitions = false;
            continue;
        }

        if (readDefinitions)
        {
            std::istringstream iss(line);
            std::string symbol;
            int address;
            iss >> symbol >> address;
            globalDefinitionTable[symbol] = address;
        }
        else
        {
            std::istringstream iss(line);
            std::string symbol;
            iss >> symbol;
            int ref;
            while (iss >> ref)
            {
                usageTable1[symbol].push_back(ref);
            }
        }
    }

    // Lê as tabelas de definições e uso do segundo módulo
    readDefinitions = false;
    while (std::getline(input2, line))
    {
        if (line == "DEFINITION TABLE:")
        {
            readDefinitions = true;
            continue;
        }
        if (line == "USAGE TABLE:")
        {
            readDefinitions = false;
            continue;
        }

        if (readDefinitions)
        {
            std::istringstream iss(line);
            std::string symbol;
            int address;
            iss >> symbol >> address;
            globalDefinitionTable[symbol] = address;
        }
        else
        {
            std::istringstream iss(line);
            std::string symbol;
            iss >> symbol;
            int ref;
            while (iss >> ref)
            {
                usageTable2[symbol].push_back(ref);
            }
        }
    }

    input1.close();
    input2.close();

    // Agora resolvemos as referências pendentes utilizando a tabela global de definições
    for (const auto &[symbol, refs] : usageTable1)
    {
        if (globalDefinitionTable.find(symbol) == globalDefinitionTable.end())
        {
            throw std::runtime_error("Error: Undefined symbol: " + symbol);
        }
        int symbolAddress = globalDefinitionTable[symbol];
        for (const auto &ref : refs)
        {
            // Substitua o XX pelo endereço do símbolo
        }
    }

    for (const auto &[symbol, refs] : usageTable2)
    {
        if (globalDefinitionTable.find(symbol) == globalDefinitionTable.end())
        {
            throw std::runtime_error("Error: Undefined symbol: " + symbol);
        }
        int symbolAddress = globalDefinitionTable[symbol];
        for (const auto &ref : refs)
        {
            // Substitua o XX pelo endereço do símbolo
        }
    }

    output.close();
}