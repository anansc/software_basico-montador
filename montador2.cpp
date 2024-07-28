#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <regex>
#include <algorithm>
#include <stdexcept>

class Montador
{
public:
    void preprocess(const std::string &inputFile, const std::string &outputFile)
    {
        std::ifstream input(inputFile);
        std::ofstream output(outputFile);
        std::string line;
        std::unordered_map<std::string, int> equMap;
        std::vector<std::string> lines;
        bool processNextLine = true;

        // Primeiro passo: Processar todas as diretivas EQU
        while (std::getline(input, line))
        {
            line = removeComments(line);
            line = removeExtraSpaces(line);
            if (line.empty())
                continue;

            if (std::regex_search(line, std::regex("\\bEQU\\b", std::regex_constants::icase)))
            {
                processEqu(line, equMap);
            }
            else
            {
                lines.push_back(line);
            }
        }

        // Substituir valores de EQU nas linhas
        for (auto &l : lines)
        {
            l = replaceEqu(l, equMap);
        }

        // Processar diretivas IF
        auto it = lines.begin();
        while (it != lines.end())
        {
            std::string currentLine = *it;

            if (std::regex_search(currentLine, std::regex("\\bIF\\b", std::regex_constants::icase)))
            {
                // Processar diretiva IF e verificar a necessidade de incluir a linha seguinte
                try
                {
                    // Armazena a próxima linha
                    std::string nextLine;
                    if (++it != lines.end())
                    {
                        nextLine = *it;
                    }
                    else
                    {
                        throw std::runtime_error("Error: No line following IF directive.");
                    }

                    processIf(currentLine, equMap, output, nextLine);
                }
                catch (const std::runtime_error &e)
                {
                    std::cerr << e.what() << std::endl;
                    return; // Interromper a execução se houver erro
                }
            }
            else
            {
                output << currentLine << std::endl;
            }
            ++it;
        }
    }

    void assemble(const std::string &inputFile, const std::string &outputFile)
    {
        std::ifstream input(inputFile);
        std::ofstream output(outputFile);
        std::string line;
        int locationCounter = 0;
        std::unordered_map<std::string, int> symbolTable;
        std::unordered_set<std::string> labelsInCurrentLine;
        bool hasBeginEnd = false;

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

                                                            // Print the values
                                                            std::cout << "Tokens: ";
                                                            for (const auto &token : tokens)
                                                            {
                                                                std::cout << token << " ";
                                                            }
                                                            std::cout << std::endl;

                                                            std::cout << "Label: " << label << std::endl;
                                                            std::cout << "Opcode: " << opcode << std::endl;

                                                            std::cout << "Operands: ";
                                                            for (const auto &operand : operands)
                                                            {
                                                                std::cout << operand << " ";
                                                            }
                                                            std::cout << std::endl;

            if (!label.empty())
            {
                if (!isValidLabel(label))
                {
                    throw std::runtime_error("Error: Invalid label: " + label);
                }
                if (labelsInCurrentLine.find(label) != labelsInCurrentLine.end())
                {
                    throw std::runtime_error("Error: Duplicate label in the same line: " + label);
                }
                labelsInCurrentLine.insert(label);

                if (symbolTable.find(label) != symbolTable.end())
                {
                    throw std::runtime_error("Error: Redefinition of label: " + label);
                }
                symbolTable[label] = locationCounter;
            }

            if (!isValidOpcode(opcode))
            {
                throw std::runtime_error("Error: Invalid opcode: " + opcode);
            }
            if (!hasCorrectNumberOfOperands(opcode, operands.size()))
            {
                throw std::runtime_error("Error: Incorrect number of operands for opcode: " + opcode);
            }

            if (opcode == "BEGIN" || opcode == "END")
            {
                hasBeginEnd = true;
            }

            // Gerar código objeto diretamente
            if (opcode == "SPACE")
            {
                output << "00 ";
                locationCounter++;
            }
            else if (opcode == "CONST")
            {
                output << operands[0] << " ";
                locationCounter++;
            }
            else
            {
                int opcodeValue = getOpcodeValue(opcode);
                output << opcodeValue << " ";
                locationCounter++;

                for (const auto &operand : operands)
                {
                    if (symbolTable.find(operand) != symbolTable.end())
                    {
                        output << symbolTable[operand] << " ";
                    }
                    else if (isValidImmediateValue(operand))
                    {
                        output << operand << " "; // Para valores imediatos
                    }
                    else
                    {
                        throw std::runtime_error("Error: Undefined symbol: " + operand);
                    }
                    locationCounter++;
                }
            }
        }

        if (!hasBeginEnd)
        {
            throw std::runtime_error("Error: Missing BEGIN or END directive.");
        }
    }

private:
    std::string removeComments(const std::string &line)
    {
        size_t commentPos = line.find(';');
        if (commentPos != std::string::npos)
        {
            return line.substr(0, commentPos);
        }
        return line;
    }

    bool isValidImmediateValue(const std::string &operand)
    {
        for (char c : operand)
        {
            if (!isdigit(c) && c != '-')
            {
                return false;
            }
        }
        return true;
    }

    void processEqu(const std::string &line, std::unordered_map<std::string, int> &equMap)
    {
        std::istringstream iss(line);
        std::string label, equ, value;
        std::getline(iss, label, ':'); // Ler o rótulo incluindo os dois pontos
        iss >> equ >> value;
        label = removeExtraSpaces(label); // Remove espaços extras ao redor do rótulo
        if (!label.empty())
        {
            equMap[label] = std::stoi(value);
        }
    }

    void processIf(const std::string &line, const std::unordered_map<std::string, int> &equMap, std::ofstream &output, const std::string &nextLine)
    {
        std::istringstream iss(line);
        std::string directive, conditionStr;
        iss >> directive >> conditionStr;
        conditionStr = removeExtraSpaces(conditionStr);

        try
        {
            int condition = std::stoi(conditionStr); // Converte a condição para um valor numérico

            if (condition == 1)
            {
                output << nextLine << std::endl;
            }
        }
        catch (const std::invalid_argument &)
        {
            throw std::runtime_error("Error: Invalid condition in IF directive: " + conditionStr);
        }
        catch (const std::out_of_range &)
        {
            throw std::runtime_error("Error: Condition out of range in IF directive: " + conditionStr);
        }
    }

    std::string replaceEqu(const std::string &line, const std::unordered_map<std::string, int> &equMap)
    {
        std::string result = line;
        for (const auto &[label, value] : equMap)
        {
            std::string labelWithColon = label + ":";
            result = std::regex_replace(result, std::regex("\\b" + labelWithColon + "\\b"), std::to_string(value));
            result = std::regex_replace(result, std::regex("\\b" + label + "\\b"), std::to_string(value));
        }
        return result;
    }

    std::string removeExtraSpaces(const std::string &line)
    {
        std::string result;
        std::unique_copy(line.begin(), line.end(), std::back_insert_iterator<std::string>(result),
                         [](char a, char b)
                         { return isspace(a) && isspace(b); });
        return result;
    }

    std::vector<std::string> tokenize(const std::string &line)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream iss(line);
        char ch;

        while (iss.get(ch))
        {
            if (ch == ' ' || ch == '\t')
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

    void parseTokens(const std::vector<std::string> &tokens, std::string &label, std::string &opcode, std::vector<std::string> &operands)
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

    bool isValidLabel(const std::string &label)
    {
        return std::regex_match(label, std::regex("^[A-Za-z][A-Za-z0-9]*$"));
    }

    bool isValidOpcode(const std::string &opcode)
    {
        static const std::unordered_set<std::string> validOpcodes = {
            "ADD", "SUB", "MULT", "DIV", "JMP", "JMPN", "JMPP", "JMPZ", "COPY",
            "LOAD", "STORE", "INPUT", "OUTPUT", "STOP"};

        return validOpcodes.find(opcode) != validOpcodes.end();
    }

    bool hasCorrectNumberOfOperands(const std::string &opcode, size_t numOperands)
    {
        static const std::unordered_map<std::string, size_t> opcodeOperandCount = {
            {"ADD", 1}, {"SUB", 1}, {"MULT", 1}, {"DIV", 1}, {"JMP", 1}, {"JMPN", 1}, {"JMPP", 1}, {"JMPZ", 1}, {"COPY", 2}, {"LOAD", 1}, {"STORE", 1}, {"INPUT", 1}, {"OUTPUT", 1}, {"STOP", 0}};

        auto it = opcodeOperandCount.find(opcode);
        if (it != opcodeOperandCount.end())
        {
            return it->second == numOperands;
        }
        return false;
    }

    int getOpcodeValue(const std::string &opcode)
    {
        static const std::unordered_map<std::string, int> opcodeValues = {
            {"ADD", 1}, {"SUB", 2}, {"MULT", 3}, {"DIV", 4}, {"JMP", 5}, {"JMPN", 6}, {"JMPP", 7}, {"JMPZ", 8}, {"COPY", 9}, {"LOAD", 10}, {"STORE", 11}, {"INPUT", 12}, {"OUTPUT", 13}, {"STOP", 14}};

        auto it = opcodeValues.find(opcode);
        if (it != opcodeValues.end())
        {
            return it->second;
        }
        throw std::runtime_error("Error: Invalid opcode: " + opcode);
    }

    int getOpcodeSize(const std::string &opcode, size_t numOperands)
    {
        if (opcode == "COPY")
        {
            return 3; // COPY tem dois operandos
        }
        return 2; // Todas as outras instruções têm um operando
    }
};

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " -p input.asm | -o input.pre" << std::endl;
        return 1;
    }

    Montador montador;
    std::string mode = argv[1];
    std::string inputFile = argv[2];

    try
    {
        if (mode == "-p")
        {
            montador.preprocess(inputFile, inputFile + ".pre");
        }
        else if (mode == "-o")
        {
            montador.assemble(inputFile, inputFile + ".obj");
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