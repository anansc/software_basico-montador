#include "preprocessor.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <regex>
#include <algorithm>
#include <stdexcept>

void Preprocessor::preprocess(const std::string &inputFile, const std::string &outputFile)
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

void Preprocessor::processEqu(const std::string &line, std::unordered_map<std::string, int> &equMap)
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

std::string Preprocessor::replaceEqu(const std::string &line, const std::unordered_map<std::string, int> &equMap)
{
    std::string result = line;
    for (const auto &pair : equMap)
    {
        size_t pos = result.find(pair.first);
        if (pos != std::string::npos)
        {
            result.replace(pos, pair.first.length(), std::to_string(pair.second));
        }
    }
    return result;
}

void Preprocessor::processIf(const std::string &line, const std::unordered_map<std::string, int> &equMap, std::ofstream &output, const std::string &nextLine)
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

std::string Preprocessor::removeComments(const std::string &line)
{
    return Utils::removeComments(line);
}

std::string Preprocessor::removeExtraSpaces(const std::string &line)
{
    return Utils::removeExtraSpaces(line);
}
