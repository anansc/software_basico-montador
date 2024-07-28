#include "token.h"
#include <sstream>

std::vector<std::string> Token::tokenize(const std::string &line)
{
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;
    while (iss >> token)
    {
        tokens.push_back(token);
    }
    return tokens;
}

void Token::parseTokens(const std::vector<std::string> &tokens, std::string &label, std::string &opcode, std::vector<std::string> &operands)
{
    label.clear();
    opcode.clear();
    operands.clear();

    size_t index = 0;
    if (tokens.size() > 0 && tokens[0].back() == ':')
    {
        label = tokens[0].substr(0, tokens[0].length() - 1);
        index++;
    }

    if (index < tokens.size())
    {
        opcode = tokens[index];
        index++;
    }

    while (index < tokens.size())
    {
        operands.push_back(tokens[index]);
        index++;
    }
}
