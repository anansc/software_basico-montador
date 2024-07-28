#include "utils.h"
#include <regex>

std::string Utils::removeComments(const std::string &line)
{
    size_t commentPos = line.find(';');
    if (commentPos != std::string::npos)
    {
        return line.substr(0, commentPos);
    }
    return line;
}

std::string Utils::removeExtraSpaces(const std::string &line)
{
    std::regex extraSpaces("\\s+");
    return std::regex_replace(line, extraSpaces, " ");
}

std::string Utils::replaceExtension(const std::string &filename, const std::string &newExtension) {
    size_t dotPosition = filename.find_last_of(".");
    if (dotPosition == std::string::npos) {
        return filename + newExtension;
    } else {
        return filename.substr(0, dotPosition) + newExtension;
    }
}