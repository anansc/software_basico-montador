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
        "LOAD", "STORE", "INPUT", "OUTPUT", "STOP"};

    return validOpcodes.find(opcode) != validOpcodes.end();
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

// Implementação da classe Assembler
void Assembler::assemble(const std::string &inputFile, const std::string &finalOutputFile)
{
    std::ifstream input(inputFile);
    std::ofstream tempOutput("temp.obj");
    std::ofstream finalOutput(finalOutputFile);
    std::string line;
    int locationCounter = 0;
    std::unordered_map<std::string, int> symbolTable;                    // Tabela de símbolos
    std::unordered_map<std::string, std::vector<int>> pendingReferences; // Referências pendentes

    // Primeira Passagem: Montagem inicial
    while (std::getline(input, line))
    {
        line = removeComments(line);
        line = removeExtraSpaces(line);
        if (line.empty())
            continue;

        std::vector<std::string> tokens = tokenize(line);
        std::string label, opcode;
        std::vector<std::string> operands;

        parseTokens(tokens, label, opcode, operands);

        // Processar rótulo
        if (!label.empty())
        {
            if (!isValidLabel(label))
                throw std::runtime_error("Error: Invalid label: " + label);

            if (symbolTable.find(label) != symbolTable.end())
                throw std::runtime_error("Error: Redefinition of label: " + label);

            // Registra o rótulo na tabela de símbolos com o valor atual do localCounter
            symbolTable[label] = locationCounter;
        }

        // Processar opcode e operandos
        if (opcode == "SPACE")
        {
            tempOutput << "00 ";
            locationCounter++;
        }
        else if (opcode == "CONST")
        {
            tempOutput << operands[0] << " ";
            locationCounter++;
        }
        else
        {
            int opcodeValue = getOpcodeValue(opcode);
            tempOutput << opcodeValue << " ";

            for (const auto &operand : operands)
            {
                if (symbolTable.find(operand) != symbolTable.end())
                {
                    tempOutput << symbolTable[operand] << " ";
                }
                else if (isValidImmediateValue(operand))
                {
                    tempOutput << operand << " ";
                }
                else
                {
                    // Adiciona referência pendente
                    pendingReferences[operand].push_back(locationCounter);
                    tempOutput << "XX "; // Coloca zero para referências não resolvidas
                }
                locationCounter++;
            }
        }
    }

    input.close();
    tempOutput.close();

    // Segunda Passagem: Substituição de referências pendentes
    std::ifstream tempInput("temp.obj");
    locationCounter = 0;
    while (std::getline(tempInput, line))
    {
        std::istringstream iss(line);
        std::string token;
        while (iss >> token)
        {
            if (token == "XX")
            { // Verifica se é uma referência pendente
                bool resolved = false;
                for (auto &[operand, refs] : pendingReferences)
                {
                    if (symbolTable.find(operand) != symbolTable.end())
                    {
                        int symbolValue = symbolTable[operand];
                        for (int refPos : refs)
                        {
                            if (locationCounter == refPos)
                            {
                                finalOutput << symbolValue << " ";
                                resolved = true;
                                break;
                            }
                        }
                        if (resolved) break;
                    }
                }
                if (!resolved)
                {
                    finalOutput << "00 "; // Caso não tenha sido resolvido, coloca zero
                }
            }
            else
            {
                finalOutput << token << " ";
            }
            locationCounter++;
        }
        finalOutput << std::endl;
    }

    tempInput.close();
    finalOutput.close();
}
