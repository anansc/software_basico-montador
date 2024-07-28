#ifndef UTILS_H
#define UTILS_H

#include <string>

class Utils
{
public:
    static std::string removeComments(const std::string &line);
    static std::string removeExtraSpaces(const std::string &line);
    static std::string replaceExtension(const std::string &filename, const std::string &newExtension);
};

#endif // UTILS_H
