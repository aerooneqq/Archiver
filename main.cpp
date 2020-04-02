#include <iostream>

#include <time.h> 

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

int main()
{
    int filesCount = 9;
    std::string* fileNames = new std::string[9] {"6.jpg", "2.docx", "1.txt", "3.pptx", "4.pdf", 
        "5.dll", "7.jpg", "8.bmp", "9.bmp"};

    int archiverLength = 4;
    Archiver** archivers = new Archiver*[4] {new LZ77Archiver(4 * 1024, 1024), new Shannon(), 
                                             new LZ77Archiver(8 * 1024, 2 * 1024), new LZ77Archiver(16 * 1024, 4 * 1024)};

    for (int i = 0; i < filesCount; ++i)
    {
        std::string fileName = "./DATA/" + fileNames[i];
        std::cout << fileName << "\n";
        for (int j = 0; j < archiverLength; ++j)
        {
            std::string res = archivers[j]->GetShortName();

            clock_t start = clock();
            archivers[j]->Archive(fileName, fileName + "." + archivers[j]->GetShortName());
            clock_t end = clock();
            
            std::cout << "Archiving time: " << (double)(end - start) / CLOCKS_PER_SEC;

            start = clock();
            archivers[j]->Dearchive(fileName + "." + archivers[j]->GetShortName(), fileName + ".un" + archivers[j]->GetShortName());
            end = clock();

            std::cout << "Dearchiving time: " << (double)(end - start) / CLOCKS_PER_SEC;

            std::cout << compareFiles(fileName, fileName + ".un" + archivers[j]->GetShortName()) << "\n";
        }
    }
    
    delete fileNames;
}