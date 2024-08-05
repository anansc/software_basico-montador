#include "assembler.h"
#include "token.h"
#include "utils.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <iterator>
#include <vector>
#include <string>
#include <cctype>
#include <stack>

// Funções utilitárias
std::string Assembler::removeComments(const std::string &line)
{
    size_t commentPos = line.find(';');
    if (commentPos != std::string::npos)
    {
        return line.substr(0, commentPos);
    }
    return line;
}

std::string Assembler::removeExtraSpaces(const std::string &line)
{
    std::string result;
    std::unique_copy(line.begin(), line.end(), std::back_insert_iterator<std::string>(result),
                     [](char a, char b)
                     { return isspace(a) && isspace(b); });
    return result;
}

std::vector<std::string> Assembler::tokenize(const std::string &line)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(line);
    char ch;

    while (iss.get(ch))
    {
        if (ch == ' ' || ch == '\t' || ch == ',')
        {
            if (!token.empty())
            {
                tokens.push_back(token);
                token.clear();
            }
        }
        else
        {
            token.push_back(ch);
        }
    }

    if (!token.empty())
    {
        tokens.push_back(token);
    }

    return tokens;
}

void Assembler::parseTokens(const std::vector<std::string> &tokens, std::string &label, std::string &opcode, std::vector<std::string> &operands)
{
    if (tokens.empty())
        return;

    size_t i = 0;

    if (tokens[i].back() == ':')
    {
        label = tokens[i].substr(0, tokens[i].size() - 1);
        ++i;
    }

    if (i < tokens.size())
    {
        opcode = tokens[i++];
    }

    while (i < tokens.size())
    {
        operands.push_back(tokens[i++]);
    }
}

bool Assembler::isValidLabel(const std::string &label)
{
    return std::regex_match(label, std::regex("^[A-Za-z_][A-Za-z0-9_]*$"));
}

bool Assembler::isValidOpcode(const std::string &opcode)
{
    static const std::unordered_set<std::string> validOpcodes = {
        "ADD", "SUB", "MULT", "DIV", "JMP", "JMPN", "JMPP", "JMPZ", "COPY",
        "LOAD", "STORE", "INPUT", "OUTPUT", "STOP", "SECTION", "SPACE", "CONST", "BEGIN", "END"};

    return validOpcodes.find(opcode) != validOpcodes.end();
}

bool Assembler::isValidDirective(const std::string &directive)
{
    static const std::unordered_set<std::string> validDirectives = {
        "BEGIN", "END", "EXTERN", "PUBLIC"};

    return validDirectives.find(directive) != validDirectives.end();
}

bool Assembler::hasCorrectNumberOfOperands(const std::string &opcode, size_t numOperands)
{
    static const std::unordered_map<std::string, size_t> opcodeOperands = {
        {"ADD", 1}, {"SUB", 1}, {"MULT", 1}, {"DIV", 1}, {"JMP", 1}, {"JMPN", 1}, {"JMPP", 1}, {"JMPZ", 1}, {"COPY", 2}, {"LOAD", 1}, {"STORE", 1}, {"INPUT", 1}, {"OUTPUT", 1}, {"STOP", 0}, {"SPACE", 0}, {"CONST", 1}, {"BEGIN", 0}, {"END", 0}};

    auto it = opcodeOperands.find(opcode);
    return it != opcodeOperands.end() && it->second == numOperands;
}

int Assembler::getOpcodeValue(const std::string &opcode)
{
    static const std::unordered_map<std::string, int> opcodeValues = {
        {"ADD", 1}, {"SUB", 2}, {"MULT", 3}, {"DIV", 4}, {"JMP", 5}, {"JMPN", 6}, {"JMPP", 7}, {"JMPZ", 8}, {"COPY", 9}, {"LOAD", 10}, {"STORE", 11}, {"INPUT", 12}, {"OUTPUT", 13}, {"STOP", 14}};

    auto it = opcodeValues.find(opcode);
    if (it != opcodeValues.end())
    {
        return it->second;
    }
    throw std::runtime_error("Invalid opcode: " + opcode);
}

bool Assembler::isValidImmediateValue(const std::string &operand)
{
    return std::regex_match(operand, std::regex("^\\d+$"));
}
struct SymbolInfo
{
    int address;
    bool isExtern;
    bool isResolved;
};

// Função para processar a diretiva "BEGIN"
void Assembler::processBeginDirective()
{
    std::cout << "Found BEGIN directive." << std::endl;
    hasBegin = true;
    symbolTable[label] = {locationCounter, false, true};
    definitionTable[label] = locationCounter;
    continue;
}

// Função para processar a diretiva "EXTERN"
void Assembler::processExternDirective()
{
    std::cout << "Found EXTERN directive." << std::endl;
    if (symbolTable.find(label) != symbolTable.end())
        throw std::runtime_error("Error: Redefinition of symbol: " + label);

    symbolTable[label] = {locationCounter, true};
    continue;
}

// Função para processar a diretiva "SPACE"
void Assembler::processSpaceDirective()
{
    symbolTable[label] = {locationCounter, false}; // Endereço atual e não externo
    std::cout << "Processing SPACE directive." << std::endl;
    for (const auto &operand : operands)
    {
        std::cout << "Operand: " << operand << std::endl;
    }
    if (!operands.empty())
    {
        if (!isValidImmediateValue(operands[0]))
            throw std::runtime_error("Error: Invalid operand for SPACE directive: " + operands[0]);
        int spaceSize = std::stoi(operands[0]);
        for (int i = 0; i < spaceSize; ++i)
        {
            tempOutput << "00 ";
            locationCounter++;
        }
    }
    else
    {
        throw std::runtime_error("Error: Missing operand for SPACE directive.");
    }
}

// Função para processar a diretiva "STOP"
void Assembler::processStopDirective()
{
    std::cout << "Processing STOP directive." << std::endl;
    int opcodeValue = getOpcodeValue(opcode);
    tempOutput << opcodeValue << " ";
    locationCounter++;
}

// Função para processar a diretiva "CONST"
void Assembler::processConstDirective()
{
    std::cout << "Processing CONST directive." << std::endl;
    tempOutput << operands[0] << " ";
    symbolTable[label] = {locationCounter, false}; // Endereço atual e não externo
    definitionTable[label] = locationCounter;
    locationCounter++;
}

// Função para processar instruções de expressão
void Assembler::processExpressionInstruction(line, opRegex)
{
    std::cout << "Processing EXPRESSION instruction for " << opcode << std::endl;
    int opcodeValue = getOpcodeValue(opcode);
    tempOutput << opcodeValue << " ";
    locationCounter++;

    std::cout << "operands: " << operands[0] << " and " << operands[2] << std::endl;

    if (symbolTable.find(operands[0]) != symbolTable.end())
    {
        std::cout << "Processing expression token adress: " << symbolTable[operands[0]].address << std::endl;
        int op_0 = symbolTable[operands[0]].address;
        int op_2 = std::stoi(operands[2]);
        char op = operands[1][0];
        switch (op)
        {
        case '+':
            tempOutput << op_0 + op_2 << " ";
            break;
        case '-':
            tempOutput << op_0 - op_2 << " ";
            break;
        case '*':
            tempOutput << op_0 * op_2 << " ";
            break;
        case '/':
            if (op_0 % op_2 != 0)
            {
                throw std::runtime_error("Error: Invalid division: " + std::to_string(op_0) + " / " + std::to_string(op_2));
            }
            tempOutput << op_0 / op_2 << " ";
            break;
        default:
            throw std::runtime_error("Error: Invalid operator: " + std::string(1, op));
        }
        if (symbolTable[operands[0]].isExtern)
            usageTable[operands[0]].push_back(locationCounter);
    }
    else
    {
        std::cout << "Adding pending reference for EXPRESSION operands: " << operands[0] << std::endl;
        // Adiciona referência pendente
        pendingReferences[operands[0]].push_back(locationCounter);
        std::cout << "EXPRESSION operands: " << operands[0] << " and " << operands[2] << std::endl;
        std::cout << "EXPRESSION operator: " << operands[1] << std::endl;
        if (symbolTable[operands[0]].isExtern)
            usageTable[operands[0]].push_back(locationCounter);
        tempOutput << "EXP" << operands[1] << operands[2] << " "; // Coloca XX para referências não resolvidas
    }
    locationCounter++;
}

void Assembler::assemble(const std::string &inputFile, const std::string &finalOutputFile)
{
    std::ifstream input(inputFile);
    std::ostringstream tempOutput;
    std::ofstream finalOutput(finalOutputFile);
    std::string line;
    int locationCounter = 0;
    std::unordered_map<std::string, SymbolInfo> symbolTable;             // Tabela de símbolos com informações adicionais
    std::unordered_map<std::string, int> definitionTable;                // Tabela de definições
    std::unordered_map<std::string, std::vector<int>> usageTable;        // Tabela de uso
    std::unordered_map<std::string, std::vector<int>> pendingReferences; // Referências pendentes
    bool hasBegin = false;
    bool hasEnd = false;

    // Primeira Passagem: Montagem inicial
    std::cout << "First pass:" << std::endl;
    while (std::getline(input, line))
    {
        line = removeComments(line);
        line = removeExtraSpaces(line);
        if (line.empty())
            continue;

        std::vector<std::string> tokens = tokenize(line);
        std::string label, opcode;
        std::vector<std::string> operands;
        std::regex opRegex("[+\\-*/]");

        parseTokens(tokens, label, opcode, operands);

        // Printar a tabela de símbolos
        std::cout << "\n\n Symbol Table:" << std::endl;
        for (const auto &symbol : symbolTable)
        {
            std::cout << "Label: " << symbol.first << ", Address: " << symbol.second.address << ", Extern: " << symbol.second.isExtern << std::endl;
        }
        // Printar tabela de definições
        std::cout << "\n\n Definition Table:" << std::endl;
        for (const auto &symbol : definitionTable)
        {
            std::cout << "Label: " << symbol.first << ", Address: " << symbol.second << std::endl;
        }
        // Printar tabela de uso
        std::cout << "\n\n Usage Table:" << std::endl;
        for (const auto &symbol : usageTable)
        {
            std::cout << "Label: " << symbol.first << ", Address: ";
            for (const auto &address : symbol.second)
            {
                std::cout << address << " ";
            }
            std::cout << std::endl;
        }
        // Printar referências pendentes
        std::cout << "\n\n Pending References:" << std::endl;
        for (const auto &symbol : pendingReferences)
        {
            std::cout << "Label: " << symbol.first << ", Address: ";
            for (const auto &address : symbol.second)
            {
                std::cout << address << " ";
            }
            std::cout << std::endl;
        }

        // Processar rótulo
        if (!label.empty())
        {
            if (!isValidLabel(label))
                throw std::runtime_error("Error: Invalid label: " + label);

            std::cout << "Processing label: " << label << std::endl;

            if (opcode == "BEGIN")
            {
                std::cout << "Found BEGIN directive." << std::endl;
                hasBegin = true;
                symbolTable[label] = {locationCounter, false, true};
                definitionTable[label] = locationCounter;
                continue;
            }
            else if (opcode == "EXTERN")
            {
                std::cout << "Found EXTERN directive." << std::endl;
                if (symbolTable.find(label) != symbolTable.end())
                    throw std::runtime_error("Error: Redefinition of symbol: " + label);

                symbolTable[label] = {locationCounter, true}; // Endereço 0 e externo
                continue;                                     // Não processa como instrução
            }
            else if (opcode == "SPACE")
            {
                symbolTable[label] = {locationCounter, false}; // Endereço atual e não externo
                std::cout << "Processing SPACE directive." << std::endl;
                for (const auto &operand : operands)
                {
                    std::cout << "Operand: " << operand << std::endl;
                }
                if (!operands.empty())
                {
                    if (!isValidImmediateValue(operands[0]))
                        throw std::runtime_error("Error: Invalid operand for SPACE directive: " + operands[0]);
                    int spaceSize = std::stoi(operands[0]);
                    for (int i = 0; i < spaceSize; ++i)
                    {
                        tempOutput << "00 ";
                        locationCounter++;
                    }
                }
                else
                {
                    throw std::runtime_error("Error: Missing operand for SPACE directive.");
                }
            }
            else if (opcode == "STOP")
            {
                std::cout << "Processing STOP directive." << std::endl;
                int opcodeValue = getOpcodeValue(opcode);
                tempOutput << opcodeValue << " ";
                locationCounter++;
            }
            else if (opcode == "CONST")
            {
            }
            else if (std::regex_search(line, opRegex))
            {
                std::cout << "Processing EXPRESSION instruction for " << opcode << std::endl;
                int opcodeValue = getOpcodeValue(opcode);
                tempOutput << opcodeValue << " ";
                locationCounter++;

                std::cout << "operands: " << operands[0] << " and " << operands[2] << std::endl;

                if (symbolTable.find(operands[0]) != symbolTable.end())
                {
                    std::cout << "Processing expression token adress: " << symbolTable[operands[0]].address << std::endl;
                    int op_0 = symbolTable[operands[0]].address;
                    int op_2 = std::stoi(operands[2]);
                    char op = operands[1][0];
                    switch (op)
                    {
                    case '+':
                        tempOutput << op_0 + op_2 << " ";
                        break;
                    case '-':
                        tempOutput << op_0 - op_2 << " ";
                        break;
                    case '*':
                        tempOutput << op_0 * op_2 << " ";
                        break;
                    case '/':
                        if (op_0 % op_2 != 0)
                        {
                            throw std::runtime_error("Error: Invalid division: " + std::to_string(op_0) + " / " + std::to_string(op_2));
                        }
                        tempOutput << op_0 / op_2 << " ";
                        break;
                    default:
                        throw std::runtime_error("Error: Invalid operator: " + std::string(1, op));
                    }
                    if (symbolTable[operands[0]].isExtern)
                        usageTable[operands[0]].push_back(locationCounter);
                }
                else
                {
                    std::cout << "Adding pending reference for EXPRESSION operands: " << operands[0] << std::endl;
                    // Adiciona referência pendente
                    pendingReferences[operands[0]].push_back(locationCounter);
                    std::cout << "EXPRESSION operands: " << operands[0] << " and " << operands[2] << std::endl;
                    std::cout << "EXPRESSION operator: " << operands[1] << std::endl;
                    if (symbolTable[operands[0]].isExtern)
                        usageTable[operands[0]].push_back(locationCounter);
                    tempOutput << "EXP" << operands[1] << operands[2] << " "; // Coloca XX para referências não resolvidas
                }
                locationCounter++;
            }
            else
            {
                std::cout << "Processing label without specific directive." << std::endl;
                symbolTable[label] = {locationCounter, false}; // Endereço atual e não externo
            }
        }
        else if (opcode == "END")
        {
            std::cout << "Found END directive." << std::endl;
            hasEnd = true;
            continue; // Não processa como instrução
        }
        else if (opcode == "PUBLIC")
        {
            std::cout << "Found PUBLIC directive." << std::endl;
            for (const auto &operand : operands)
            {
                definitionTable[operand] = {};
            }
            continue; // Não processa como instrução
        }
        else if (std::regex_search(line, opRegex))
        {
            std::cout << "Processing EXPRESSION instruction for " << opcode << std::endl;
            int opcodeValue = getOpcodeValue(opcode);
            tempOutput << opcodeValue << " ";
            locationCounter++;

            std::cout << "operands: " << operands[0] << " and " << operands[2] << std::endl;

            if (symbolTable.find(operands[0]) != symbolTable.end())
            {
                std::cout << "Processing expression token adress: " << symbolTable[operands[0]].address << std::endl;
                int op_0 = symbolTable[operands[0]].address;
                int op_2 = std::stoi(operands[2]);
                char op = operands[1][0];
                switch (op)
                {
                case '+':
                    tempOutput << op_0 + op_2 << " ";
                    break;
                case '-':
                    tempOutput << op_0 - op_2 << " ";
                    break;
                case '*':
                    tempOutput << op_0 * op_2 << " ";
                    break;
                case '/':
                    if (op_0 % op_2 != 0)
                    {
                        throw std::runtime_error("Error: Invalid division: " + std::to_string(op_0) + " / " + std::to_string(op_2));
                    }
                    tempOutput << op_0 / op_2 << " ";
                    break;
                default:
                    throw std::runtime_error("Error: Invalid operator: " + std::string(1, op));
                }
                if (symbolTable[operands[0]].isExtern)
                    usageTable[operands[0]].push_back(locationCounter);
            }
            else
            {
                std::cout << "Adding pending reference for EXPRESSION operands: " << operands[0] << std::endl;
                // Adiciona referência pendente
                pendingReferences[operands[0]].push_back(locationCounter);
                std::cout << "EXPRESSION operands: " << operands[0] << " and " << operands[2] << std::endl;
                std::cout << "EXPRESSION operator: " << operands[1] << std::endl;
                if (symbolTable[operands[0]].isExtern)
                    usageTable[operands[0]].push_back(locationCounter);
                tempOutput << "EXP" << operands[1] << operands[2] << " "; // Coloca XX para referências não resolvidas
            }
            locationCounter++;
        }
        else
        {
            std::cout << "Processing general instruction: " << opcode << "in location counter: " << locationCounter << std::endl;
            int opcodeValue = getOpcodeValue(opcode);
            tempOutput << opcodeValue << " ";
            locationCounter++;

            for (const auto &operand : operands)
            {
                if (symbolTable.find(operand) != symbolTable.end())
                {
                    tempOutput << symbolTable[operand].address << " ";
                    std::cout << symbolTable[operand].isExtern << std::endl;
                    if (symbolTable[operand].isExtern)
                        usageTable[operand].push_back(locationCounter);
                }
                else if (isValidImmediateValue(operand))
                {
                    std::cout << "Processing immediate value: " << operand << std::endl;
                    tempOutput << operand << " ";
                }
                else
                {
                    std::cout << "Adding pending reference for operand: " << operand << "in location counter: " << locationCounter << std::endl;
                    // Adiciona referência pendente
                    pendingReferences[operand].push_back(locationCounter);
                    tempOutput << "XX "; // Coloca XX para referências não resolvidas
                }
                locationCounter++;
            }
        }
    }

    if ((hasBegin && !hasEnd) || (!hasBegin && hasEnd))
    {
        throw std::runtime_error("Error: Missing BEGIN or END directive.");
    }

    input.close();

    // Segunda Passagem: Substituição de referências pendentes
    std::istringstream tempInput(tempOutput.str());
    locationCounter = 0;
    std::regex pattern("^EXP");
    std::cout << "Second pass: Resolving pending references." << std::endl;
    while (std::getline(tempInput, line))
    {
        std::istringstream iss(line);
        std::string token;
        while (iss >> token)
        {
            if (token == "XX")
            { // Verifica se é uma referência pendente
                bool resolved = false;
                for (const auto &[operand, refs] : pendingReferences)
                {
                    if (symbolTable.find(operand) != symbolTable.end() && !symbolTable[operand].isExtern)
                    {
                        int symbolValue = symbolTable[operand].address;
                        for (int refPos : refs)
                        {
                            if (locationCounter == refPos)
                            {
                                finalOutput << symbolValue << " ";
                                definitionTable[operand] = symbolValue;
                                std::cout << "Resolved pending reference for symbol: " << operand << " at position " << refPos << " with value " << symbolValue << std::endl;
                                resolved = true;
                                break;
                            }
                        }
                        if (resolved)
                            break;
                    }
                }
                if (!resolved)
                {
                    std::cout << "Unresolved reference at position " << locationCounter << ". Using default value 00." << std::endl;
                    finalOutput << "00 "; // Caso não tenha sido resolvido, coloca zero
                }
            }
            else if (std::regex_search(token, pattern))
            {
                bool resolved = false;
                for (const auto &[operand, refs] : pendingReferences)
                {
                    if (symbolTable.find(operand) != symbolTable.end() && !symbolTable[operand].isExtern)
                    {
                        int symbolValue = symbolTable[operand].address;
                        for (int refPos : refs)
                        {
                            if (locationCounter == refPos)
                            {
                                std::cout << "Processing expression token: " << token << std::endl;
                                std::cout << "SymbolValue: " << symbolValue << std::endl;
                                int op_0 = symbolValue;
                                int op_2 = std::stoi(token.substr(4));
                                char op = token[3];
                                switch (op)
                                {
                                case '+':
                                    finalOutput << op_0 + op_2 << " ";
                                    break;
                                case '-':
                                    finalOutput << op_0 - op_2 << " ";
                                    break;
                                case '*':
                                    finalOutput << op_0 * op_2 << " ";
                                    break;
                                case '/':
                                    if (op_0 % op_2 != 0)
                                    {
                                        throw std::runtime_error("Error: Invalid division: " + std::to_string(op_0) + " / " + std::to_string(op_2));
                                    }
                                    finalOutput << op_0 / op_2 << " ";
                                    break;
                                default:
                                    throw std::runtime_error("Error: Invalid operator: " + std::string(1, op));
                                }
                                resolved = true;
                                break;
                            }
                        }
                        if (resolved)
                            break;
                    }
                }
            }
            else
            {
                std::cout << "Writing token: " << token << std::endl;
                finalOutput << token << " ";
            }
            locationCounter++;
        }
        finalOutput << std::endl;
    }

    // Escreve a tabela de definições no final do arquivo de saída
    std::cout << "Writing definition table to output file." << std::endl;
    finalOutput << "DEFINITION TABLE:" << std::endl;
    for (const auto &[symbol, address] : definitionTable)
    {
        std::cout << "Definition: " << symbol << " " << address << std::endl;
        finalOutput << symbol << " " << address << std::endl;
    }

    // Escreve a tabela de uso no final do arquivo de saída
    std::cout << "Writing usage table to output file." << std::endl;
    finalOutput << "USAGE TABLE:" << std::endl;
    for (const auto &[symbol, refs] : usageTable)
    {
        finalOutput << symbol << " ";
        for (const auto &ref : refs)
        {
            std::cout << "Usage: " << symbol << " " << ref << std::endl;
            finalOutput << ref << " ";
        }
        finalOutput << std::endl;
    }
    finalOutput.close();
}
