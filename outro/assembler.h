#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

class Assembler {
public:
    void assemble(const std::string &inputFile, const std::string &outputFile);

private:
    struct SymbolInfo {
        int address;
        bool isExtern;
        bool isResolved;
    };

    int locationCounter;
    bool hasBegin;
    bool hasEnd;
    std::unordered_map<std::string, SymbolInfo> symbolTable;
    std::unordered_map<std::string, int> definitionTable;
    std::unordered_map<std::string, std::vector<int>> usageTable;
    std::unordered_map<std::string, std::vector<int>> pendingReferences;

    std::string removeComments(const std::string &line);
    std::string removeExtraSpaces(const std::string &line);
    std::vector<std::string> tokenize(const std::string &line);
    void parseTokens(const std::vector<std::string> &tokens, std::string &label, std::string &opcode, std::vector<std::string> &operands);
    bool isValidLabel(const std::string &label);
    bool isValidOpcode(const std::string &opcode);
    bool isValidDirective(const std::string &directive);
    bool hasCorrectNumberOfOperands(const std::string &opcode, size_t numOperands);
    int getOpcodeValue(const std::string &opcode);
    bool isValidImmediateValue(const std::string &value);

    void processLabel(const std::string &label, int locationCounter, bool isExtern, bool isResolved);
    void processBeginDirective(const std::string &label);
    void processExternDirective(const std::string &label);
    void processSpaceDirective(const std::string &label, const std::vector<std::string> &operands, std::ostringstream &tempOutput);
    void processStopDirective(std::ostringstream &tempOutput);
    void processConstDirective(const std::string &label, const std::vector<std::string> &operands, std::ostringstream &tempOutput);
    void processExpressionInstruction(const std::string &opcode, const std::vector<std::string> &operands, std::ostringstream &tempOutput);
};

#endif // ASSEMBLER_H
