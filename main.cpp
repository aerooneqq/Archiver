#include <iostream>
#include <chrono> 

#include "Archiver.h"
#include "Util.h"
#include "Shannon.h"
#include "LZ77.h"

size_t compareFiles(const std::string firstFile, const std::string secondFile)
{
    FileReader firstReader(firstFile);
    FileReader secondReader(secondFile);

    int firstReadBytes;
    int secondReadBytes;
    size_t byteIndex = 0;

    while (true)
    {
        unsigned char firstByte;
        unsigned char secondByte;

        firstByte = firstReader.ReadNextChar(firstReadBytes);
        secondByte = secondReader.ReadNextChar(secondReadBytes);

        if (firstReadBytes != secondReadBytes || firstByte != secondByte)
        {
            return ++byteIndex;
        }

        ++byteIndex;

        if (firstReadBytes == 0 && secondReadBytes == 0)
        {
             return -1;
        }
    }
}

std::chrono::nanoseconds measureExecutionTime(void (Archiver::*func)(const std::string input, const std::string out),
                                              Archiver* archiver, std::string inputFile, std::string outFile)
{   
    auto start = std::chrono::steady_clock::now();
    (archiver->*func)(inputFile, outFile);
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
}

int main()
{
    int filesCount = 9;
    std::string* fileNames = new std::string[9] {"5.dll", "7.jpg", "8.bmp", "9.bmp", "4.pdf", "1.txt", "2.docx", "6.jpg", "3.pptx" };

    int archiverLength = 4;
    Archiver** archivers = new Archiver*[4] {new Shannon(), new LZ77Archiver(4 * 1024, 1024), 
                                             new LZ77Archiver(8 * 1024, 2 * 1024), new LZ77Archiver(16 * 1024, 4 * 1024)};

    for (int i = 0; i < filesCount; ++i)
    {
        std::string fileName = "./DATA/" + fileNames[i];
        std::cout << fileName << "\n\n";
        for (int j = 0; j < archiverLength; ++j)
        {
            std::string archivedFileName =  fileName + "." + archivers[j]->GetShortName();
            std::string dearchivedFileName =  fileName + ".un" + archivers[j]->GetShortName();

            auto archiveTime = measureExecutionTime(Archiver::Archive, archivers[j], fileName, archivedFileName);
            auto dearchivedTime = measureExecutionTime(Archiver::Dearchive, archivers[j], archivedFileName, dearchivedFileName);

            std::cout << archivers[j]->GetShortName() << " " << "Archive time: " << archiveTime.count() / 1000000000.0 << "\n";
            std::cout << archivers[j]->GetShortName() << " " << "Dearchive time: " << dearchivedTime.count() / 1000000000.0 << "\n";

            std::cout << compareFiles(fileName, fileName + ".un" + archivers[j]->GetShortName()) << "\n";
        }
    }
 
    delete[] fileNames;
}