#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

class Linker {
public:
    void link(const std::string& objFile1, const std::string& objFile2, const std::string& outputFile);
    
private:
    void parseOBJFile(const std::string& filePath, std::unordered_map<std::string, int>& symbolTable,
                      std::vector<std::pair<std::string, int>>& usageTable, std::vector<int>& code,
                      std::string& relocationTable, int& moduleSize);
    void resolveReferences(std::unordered_map<std::string, int>& globalSymbolTable, std::vector<std::pair<std::string, int>>& usageTable,
                           std::vector<int>& code);
};

void Linker::link(const std::string& objFile1, const std::string& objFile2, const std::string& outputFile) {
    std::unordered_map<std::string, int> globalSymbolTable;
    std::vector<std::pair<std::string, int>> usageTable1, usageTable2;
    std::vector<int> code1, code2;
    std::string relocationTable1, relocationTable2;
    int moduleSize1 = 0, moduleSize2 = 0;

    // Parse the OBJ files and extract relevant data
    parseOBJFile(objFile1, globalSymbolTable, usageTable1, code1, relocationTable1, moduleSize1);
    parseOBJFile(objFile2, globalSymbolTable, usageTable2, code2, relocationTable2, moduleSize2);

    // Correction factors
    int correctionFactor2 = moduleSize1;

    // Resolve references in both code sets
    resolveReferences(globalSymbolTable, usageTable1, code1);
    resolveReferences(globalSymbolTable, usageTable2, code2);

    // Adjust relocation for the second module
    for (size_t i = 0; i < relocationTable2.size(); ++i) {
        if (relocationTable2[i] == '1') {
            code2[i] += correctionFactor2;
        }
    }

    // Write the linked output to a file
    std::ofstream output(outputFile);
    if (!output) {
        throw std::runtime_error("Could not open output file: " + outputFile);
    }

    for (const auto& code : code1) {
        output << code << " ";
    }
    for (const auto& code : code2) {
        output << code << " ";
    }
    output.close();
}

void Linker::parseOBJFile(const std::string& filePath, std::unordered_map<std::string, int>& symbolTable,
                          std::vector<std::pair<std::string, int>>& usageTable, std::vector<int>& code,
                          std::string& relocationTable, int& moduleSize) {
    std::ifstream input(filePath);
    if (!input) {
        throw std::runtime_error("Could not open input file: " + filePath);
    }

    std::string line;
    while (std::getline(input, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "USO") {
            std::string symbol;
            int pos;
            while (iss >> symbol >> pos) {
                usageTable.emplace_back(symbol, pos);
            }
        } else if (token == "DEF") {
            std::string symbol;
            int address;
            while (iss >> symbol >> address) {
                symbolTable[symbol] = address;
            }
        } else if (token == "REAL") {
            iss >> relocationTable;
        } else {
            int value;
            while (iss >> value) {
                code.push_back(value);
            }
            moduleSize += code.size();
        }
    }
    input.close();
}

void Linker::resolveReferences(std::unordered_map<std::string, int>& globalSymbolTable, std::vector<std::pair<std::string, int>>& usageTable,
                               std::vector<int>& code) {
    for (const auto& [symbol, pos] : usageTable) {
        if (globalSymbolTable.find(symbol) != globalSymbolTable.end()) {
            int symbolAddress = globalSymbolTable[symbol];
            code[pos] = symbolAddress;
        } else {
            throw std::runtime_error("Undefined symbol: " + symbol);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <prog1.obj> <prog2.obj>" << std::endl;
        return 1;
    }

    std::string objFile1 = argv[1];
    std::string objFile2 = argv[2];
    std::string outputFile = objFile1.substr(0, objFile1.find_last_of('.')) + ".e";

    Linker linker;
    try {
        linker.link(objFile1, objFile2, outputFile);
        std::cout << "Linked output written to " << outputFile << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
