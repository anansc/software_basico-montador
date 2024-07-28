#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <string>
#include <unordered_map>
#include <fstream>

class Preprocessor
{
public:
    void preprocess(const std::string &inputFile, const std::string &outputFile);

private:
    void processEqu(const std::string &line, std::unordered_map<std::string, int> &equMap);
    std::string replaceEqu(const std::string &line, const std::unordered_map<std::string, int> &equMap);
    void processIf(const std::string &line, const std::unordered_map<std::string, int> &equMap, std::ofstream &output, const std::string &nextLine);
    std::string removeComments(const std::string &line);
    std::string removeExtraSpaces(const std::string &line);
};

#endif // PREPROCESSOR_H
