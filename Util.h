#include <iostream>
#include <vector>
#include <fstream>
#include <queue>
#include "math.h"

std::vector<bool> getBitsFromByte(unsigned char byte)
{
    std::vector<bool> bits;

    for (int i = 7; i >= 0; --i)
    {
        unsigned char p = (unsigned char)pow(2, i);
        if (p <= byte)
        {
            bits.push_back(true);
            byte -= p;
        }
        else
        {
            bits.push_back(false);
        }
    }

    return bits;
} 

unsigned char getByteFromBits(std::vector<bool> bits)
{
    unsigned char byte = 0;
    for (int i = 0; i < bits.size(); ++i)
    {
        if (bits[i])
        {
            byte += (unsigned char)pow(2, 7 - i);
        }
    }

    return byte;
}

std::vector<bool> getBitsFromInt(int num, int bitsCount)
{
    std::vector<bool> bits;

    for (int i = bitsCount - 1; i >= 0; --i)
    {
        int p = (int)pow(2, i);
        if (p <= num)
        {
            num -= p;
            bits.push_back(true);
        }
        else
        {
            bits.push_back(false);
        }
    }

    if (num != 0)
    {
        for (int i = 0; i < bits.size(); ++i)
        {
            bits[i] = 0;
        }
    }

    return  bits;
}

unsigned char getByteFromQueue(std::queue<bool>* bits)
{
    if (bits->size() < 8)
    {
        return -1;
    }

    unsigned char byte = 0;

    for (int i = 7; i >= 0; --i)
    {
        if (bits->front())
        {
            byte += (unsigned char)(std::pow(2, i));
        }

        bits->pop();
    }

    return byte;
}

std::vector<bool> getBitsFromBytes(std::vector<unsigned char>& bytes, int count)
{
    std::vector<bool> bits;

    for (int i = 0; i < count; ++i)
    {
        std::vector<bool> tempBits = getBitsFromByte(bytes[i]);

        for (int j = 0; j < tempBits.size(); ++j)
        {
            bits.push_back(tempBits[j]);
        }
    }

    return bits;
}

int getIntFromBits(std::vector<bool>& bits, int count)
{
    int num = 0;
    for (int i = count - 1; i >= 0; --i)
    {
        if (bits[0])
        {
            num += (int)pow(2, i);
        }

        bits.erase(bits.begin());
    }

    return num;
}

class FileReader
{
private:
    std::ifstream* fileStream;

public:
    FileReader(const std::string filePath)
    {
        fileStream = new std::ifstream(filePath, std::ios::binary | std::ios::ate);
        fileStream->seekg(0, std::ios::beg);
    }

    void Reset() 
    {
        fileStream->seekg(0, std::ios::beg);
    }

    int Read(std::vector<unsigned char>* bytes, int size)
    {
        fileStream->read((char *)(&(bytes->at(0))), size);
        return fileStream->gcount();
    }

    unsigned char ReadNextChar(int& readBytes)
    {
        unsigned char nextChar = 0;
        fileStream->read((char*)(&nextChar), 1);
        readBytes = fileStream->gcount();
        
        return nextChar;
    }

    static std::vector<bool> ReadAll(std::string filePath)
    {
        std::ifstream ifstream(filePath, std::ios::binary | std::ios::ate);
        auto pos = ifstream.tellg();
        std::vector<unsigned char> bytes(pos);

        ifstream.seekg(0, std::ios::beg);
        ifstream.read((char *)(&bytes[0]), pos);

        std::vector<bool> fileBits;
        for (int i = 0; i < bytes.size(); ++i)
        {
            std::vector<bool> bits = getBitsFromByte(bytes[i]);
            for (int j = 0; j < bits.size(); ++j)
            {
                fileBits.push_back(bits[j]);
            }
        }

        return fileBits;
    }

    ~FileReader()
    {
        fileStream->close();
        delete fileStream;
    }
};

class FileWriter
{
private:
    std::ofstream* fileStream;
public:
    FileWriter(const std::string filePath)
    {
        fileStream = new std::ofstream(filePath, std::ios::binary | std::ios::out);
    }

    void Write(std::vector<unsigned char>* bytes)
    {
        fileStream->write((char *)(&bytes->at(0)), bytes->size());
    }

    void WriteByte(unsigned char byte)
    {
        fileStream->write((char*)(&byte), 1);
    }

    void WriteString(std::string str)
    {
        (*fileStream) << str;
    }

    ~FileWriter()
    {
        fileStream->flush();
        fileStream->close();
        delete fileStream;
    }
};
