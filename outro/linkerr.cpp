#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <sstream>

struct Definition {
    std::string label;
    int address;
};

struct Usage {
    std::string label;
    std::vector<int> locations;
};

void readOBJFile(const std::string& filename, std::vector<int>& code, std::unordered_map<std::string, int>& defTable, std::unordered_map<std::string, Usage>& useTable) {
    std::ifstream file(filename);
    std::string line;
    
    // Read code section
    int value;
    while (file >> value) {
        code.push_back(value);
    }
    
    // Read Definition Table
    while (std::getline(file, line) && line != "DEFINITION TABLE:") {}
    while (std::getline(file, line) && !line.empty()) {
        std::istringstream iss(line);
        std::string label;
        int address;
        iss >> label >> address;
        defTable[label] = address;
    }
    
    // Read Usage Table
    while (std::getline(file, line) && line != "USAGE TABLE:") {}
    while (std::getline(file, line) && !line.empty()) {
        std::istringstream iss(line);
        std::string label;
        int location;
        iss >> label;
        Usage usage;
        usage.label = label;
        while (iss >> location) {
            usage.locations.push_back(location);
        }
        useTable[label] = usage;
    }
}

void writeOutputFile(const std::string& filename, const std::vector<int>& code, const std::unordered_map<std::string, int>& defTable) {
    std::ofstream file(filename);
    
    // Write code section
    for (int value : code) {
        file << value << " ";
    }
    
    // Write Definition Table
    file << "\nDEFINITION TABLE:\n";
    for (const auto& [label, address] : defTable) {
        file << label << " " << address << "\n";
    }
}

void link(const std::vector<std::string>& objFiles, const std::string& outputFile) {
    std::unordered_map<std::string, int> combinedDefTable;
    std::unordered_map<std::string, Usage> combinedUseTable;
    std::vector<int> combinedCode;
    
    for (const std::string& file : objFiles) {
        std::vector<int> code;
        std::unordered_map<std::string, int> defTable;
        std::unordered_map<std::string, Usage> useTable;
        
        readOBJFile(file, code, defTable, useTable);
        
        // Combine definition tables
        for (const auto& [label, address] : defTable) {
            combinedDefTable[label] = address;
        }
        
        // Combine usage tables
        for (const auto& [label, usage] : useTable) {
            combinedUseTable[label] = usage;
        }
        
        // Combine code sections
        combinedCode.insert(combinedCode.end(), code.begin(), code.end());
    }
    
    // Resolve addresses
    for (auto& [label, usage] : combinedUseTable) {
        if (combinedDefTable.find(label) != combinedDefTable.end()) {
            int address = combinedDefTable[label];
            for (int loc : usage.locations) {
                combinedCode[loc] = address;
            }
        }
    }
    
    writeOutputFile(outputFile, combinedCode, combinedDefTable);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <output.e> <input1.obj> <input2.obj> [<inputN.obj>...]" << std::endl;
        return 1;
    }
    
    std::string outputFile = argv[1];
    std::vector<std::string> objFiles;
    for (int i = 2; i < argc; ++i) {
        objFiles.push_back(argv[i]);
    }
    
    link(objFiles, outputFile);
    return 0;
}
