#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <vector>

class Token
{
public:
    static std::vector<std::string> tokenize(const std::string &line);
    static void parseTokens(const std::vector<std::string> &tokens, std::string &label, std::string &opcode, std::vector<std::string> &operands);
};

#endif // TOKEN_H
