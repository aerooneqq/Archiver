#include <iostream>

class Archiver 
{
public:
    virtual void Archive(const std::string inputFile, const std::string outFile) = 0;
    virtual void Dearchive(const std::string inputFile, const std::string outFile) = 0;
    virtual std::string GetDescription() = 0;
    virtual std::string GetShortName() = 0;
};