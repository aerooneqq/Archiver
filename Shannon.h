#include <iostream>
#include <map>
#include <vector>
#include <queue>
#include <algorithm>
#include "math.h"

typedef unsigned long long int ll;

class Shannon : public Archiver
{
private:
    std::vector<std::pair<unsigned char, ll>>* countBytes(FileReader* fileReader)
    {
        std::map<unsigned char, ll> bytesCount;
        int readBytesCount;
        const int blockSize = 1024;
        std::vector<unsigned char>* bytes = new std::vector<unsigned char>(blockSize);

        do
        {
            readBytesCount = fileReader->Read(bytes, blockSize);
            for (int i = 0; i < readBytesCount; ++i)
            {
                if (bytesCount.find(bytes->at(i)) == bytesCount.end())
                {
                    bytesCount.insert(std::make_pair(bytes->at(i), 1));
                }
                else
                {
                    bytesCount[bytes->at(i)] += 1;
                }
            }
        }
        while (readBytesCount == blockSize);

        std::vector<std::pair<unsigned char, ll>>* bytesCountVector = new std::vector<std::pair<unsigned char, ll>>();
        
        for (auto it = bytesCount.begin(); it != bytesCount.end(); it++)
        {
            bytesCountVector->push_back(std::make_pair(it->first, it->second));
        }

        return bytesCountVector;
    } 

    static bool comparator(const std::pair<unsigned char, ll> p1, const std::pair<unsigned char, ll> p2)
    {
        return p2.second > p1.second;
    }

    std::map<unsigned char, std::vector<bool>*>* getCodes(std::vector<std::pair<unsigned char, ll>>* bytesCount)
    {
        std::sort(bytesCount->begin(), bytesCount->end(), Shannon::comparator);
        std::vector<ll>* dp = getDPVector(bytesCount);

        int n = bytesCount->size();
        std::queue<std::pair<int, int>> intervals;
        intervals.push(std::make_pair(0, n - 1));
        std::vector<std::vector<bool>*> codes(n);
        for (int i = 0; i < n; ++i)
        {
            codes[i] = new std::vector<bool>();
        }

        while (intervals.size() > 0)
        {
            std::pair<int, int> currInterval = intervals.front();
            intervals.pop();

            if (currInterval.first == currInterval.second || currInterval.second < currInterval.first)
            {
                continue; 
            }
            else if (currInterval.second - currInterval.first == 1)
            {
                codes[currInterval.first]->push_back(false);
                codes[currInterval.second]->push_back(true);
                continue;
            }

            int index = findMiddleIndex(dp, currInterval.first, currInterval.second);

            for (int i = currInterval.first; i <= index; ++i)
            {
                codes[i]->push_back(false);
            }

            for (int i = index + 1; i <= currInterval.second; ++i)
            {
                codes[i]->push_back(true);
            }

            intervals.push(std::make_pair(currInterval.first, index));
            intervals.push(std::make_pair(index + 1, currInterval.second));
        }

        std::map<unsigned char, std::vector<bool>*>* charCodes = new std::map<unsigned char, std::vector<bool>*>();
        for (int i = 0; i < bytesCount->size(); ++i)
        {
            charCodes->insert(std::make_pair(bytesCount->at(i).first, codes[i]));
        }

        return charCodes;
    }

    int findMiddleIndex(std::vector<ll>* dp, int left, int right)
    {
        int index = left;
        ll prevDelta = std::abs(getSum(dp, left, left) - getSum(dp, left + 1, right));

        for (int i = left + 1; i <= right - 1; ++i)
        {
            ll newDelta = std::abs(getSum(dp, i + 1, right) - getSum(dp, left, i));

            if (newDelta < prevDelta)
            {
                prevDelta = newDelta;
                index = i;
            }
            else
            {
                break;
            }
        }

        return index;
    }

    int getSum(std::vector<ll>* dp, int left, int right)
    {
        if (left <= 0)
        {
            return dp->at(right);
        }

        return dp->at(right) - dp->at(left - 1);
    }

    std::vector<ll>* getDPVector(std::vector<std::pair<unsigned char, ll>>* bytesCount)
    {
        std::vector<ll>* dp = new std::vector<ll>();
        dp->push_back(bytesCount->at(0).second);
        
        for (int i = 1; i < bytesCount->size(); ++i)
        {
            dp->push_back(dp->at(i - 1) + bytesCount->at(i).second);
        }

        return dp;
    }

public:
    void Archive(const std::string inputFile, const std::string outFile) override
    {
        FileReader* fileReader = new FileReader(inputFile);
        std::map<unsigned char, std::vector<bool>*>* codes = getCodes(countBytes(fileReader));
        
        std::vector<bool> resBits;

        int readBytes;
        FileWriter* fileWriter = new FileWriter(outFile);
        std::queue<bool> bits;
        std::vector<unsigned char> tempBytesToWrite;

        std::vector<bool> sizeBits = getBitsFromByte((unsigned char)codes->size()); 
        for (int i = 0; i < sizeBits.size(); ++i)
        {
            bits.push(sizeBits[i]);
        }

        for (auto it = codes->begin(); it != codes->end(); it++)
        {
            sizeBits = getBitsFromByte(it->first);

            for (int i = 0; i < sizeBits.size(); ++i)
            {
                bits.push(sizeBits[i]);
            }

            sizeBits = getBitsFromByte(it->second->size());

            for (int i = 0; i < sizeBits.size(); ++i)
            {
                bits.push(sizeBits[i]);
            }

            for (int i = 0; i < it->second->size(); ++i)
            {
                bits.push(it->second->at(i));
            }
        }

        int sz = codes->size();
        delete fileReader;
        fileReader = new FileReader(inputFile);
        do
        {
            unsigned char byte = fileReader->ReadNextChar(readBytes);
    
            if (readBytes > 0)
            {
                for (int j = 0; j < codes->at(byte)->size(); ++j)
                {
                    bits.push(codes->at(byte)->at(j));
                }
            }

            sz = bits.size();
            tempBytesToWrite.clear();
            while (bits.size() >= 8)
            {
                tempBytesToWrite.push_back(getByteFromQueue(&bits));
            }

            if (tempBytesToWrite.size() > 0)
            {
                fileWriter->Write(&tempBytesToWrite);
            }
        }
        while (readBytes > 0);

        tempBytesToWrite.clear();
        unsigned char lastByteSize = bits.size();
        unsigned char lastByte = 0;
        for (int i = 7; i >= 0; ++i)
        {
            if (bits.empty())
            {
                break;
            }

            if (bits.front())
            {
                lastByte += (int)(std::pow(2, i));
            }

            bits.pop();
        }

        tempBytesToWrite.push_back(lastByte);
        tempBytesToWrite.push_back(lastByteSize);

        fileWriter->Write(&tempBytesToWrite);

        delete fileReader;
        delete fileWriter;
    }

    struct CodeNode
    {
    public:
        CodeNode* left = nullptr;
        CodeNode* right = nullptr;
        bool isCode = false;
        unsigned char byte;
    };

    void Dearchive(const std::string filePath, const std::string dearchiveFilePath) override
    {
        FileReader* fr = new FileReader(filePath);
        std::vector<bool> bits = fr->ReadAll(filePath);

        std::vector<bool> tempBits;

        for (int i = 0; i < 8; ++i)
        {
            tempBits.push_back(bits[i]);
        }

        int encodedBytesCount = getByteFromBits(tempBits);
        if (encodedBytesCount == 0)
        {
            encodedBytesCount = 256;
        }

        int bitsIndex = 8;

        CodeNode* root = new CodeNode();
        std::map<unsigned char, std::vector<bool>> codes;
        for (int i = 0; i < encodedBytesCount; ++i)
        {
            tempBits.clear();
            for (int j = 0; j < 8; ++j)
            {
                tempBits.push_back(bits[bitsIndex++]);
            }

            unsigned char byte = getByteFromBits(tempBits);

            tempBits.clear();
            for (int j = 0; j < 8; ++j)
            {
                tempBits.push_back(bits[bitsIndex++]);
            }

            unsigned char codeLength = getByteFromBits(tempBits);

            std::vector<bool> code;
            for (unsigned char j = 0; j < codeLength; ++j)
            {
                code.push_back(bits[bitsIndex++]);
            }

            CodeNode* currNode = root;

            for (int j = 0; j < code.size(); ++j)
            {
                if (code[j])
                {
                    if (currNode->right == nullptr)
                    {
                        currNode->right = new CodeNode();
                        currNode = currNode->right;
                    }
                    else
                    {
                        currNode = currNode->right;
                    }
                }
                else
                {
                    if (currNode->left == nullptr)
                    {
                        currNode->left = new CodeNode();
                        currNode = currNode->left;
                    }
                    else
                    {
                        currNode = currNode->left;
                    }
                }
            }

            currNode->isCode = true;
            currNode->byte = byte;
        }

        tempBits.clear();
        for (int i = bits.size() - 8; i < bits.size(); ++i)
        {
            tempBits.push_back(bits[i]);
        }

        unsigned char meaningfullBitsInPreLastByte = getByteFromBits(tempBits);

        CodeNode* currNode = root;
        std::vector<unsigned char> dearchivedBytes;
        while (bitsIndex < bits.size() - (8 - meaningfullBitsInPreLastByte) - 8)
        {
            if (bits[bitsIndex++])
            {
                currNode = currNode->right;
            }
            else
            {
                currNode = currNode->left;
            }

            if (currNode->isCode)
            {
                dearchivedBytes.push_back(currNode->byte);
                currNode = root;
            }
        }

        FileWriter* fw = new FileWriter(dearchiveFilePath);
        fw->Write(&dearchivedBytes);    
    }

    std::string GetDescription() override
    {
        return "Shannon";
    }

    std::string GetShortName() override
    {
        return "shan";
    }
};