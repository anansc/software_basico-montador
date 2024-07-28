#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <string>
#include <vector>

class Assembler {
public:
    void assemble(const std::string &inputFile, const std::string &outputFile);
    std::string removeComments(const std::string &line);
    std::string removeExtraSpaces(const std::string &line);
    std::vector<std::string> tokenize(const std::string &line);
    void parseTokens(const std::vector<std::string> &tokens, std::string &objCode, std::string &label, std::vector<std::string> &operand);
    bool isValidLabel(const std::string &label);
    bool isValidOpcode(const std::string &opcode);
    bool isValidDirective(const std::string &directive);
    bool hasCorrectNumberOfOperands(const std::string &opcode, size_t operandCount);
    int getOpcodeValue(const std::string &opcode);
    bool isValidImmediateValue(const std::string &value);
};

#endif // ASSEMBLER_H
