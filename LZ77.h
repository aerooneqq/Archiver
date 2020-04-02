#include <iostream>
#include <map>
#include <vector>
#include <queue>
#include <algorithm>
#include "math.h"

struct LZ77Node
{
public:
    int offset;
    int length;
    unsigned char nextChar;

    LZ77Node(int offset, int length, unsigned char nextChar) : offset(offset), length(length), nextChar(nextChar) {}
    LZ77Node() {}
};

class LZ77Archiver : public Archiver
{
private:
    int historySize;
    int viewSize;
    std::vector<LZ77Node*> nn;

    int find(std::vector<unsigned char>& history, std::vector<unsigned char>& view, 
             std::vector<unsigned char>& subvector)
    {
        for (int i = history.size() - 1; i >= 0; --i)
        {
            if (history[i] == subvector[0])
            {
                int j;
                int ii = i;

                for (j = 0; j < std::min(history.size() - i, subvector.size()); ++j)
                {
                    if (history[ii] == subvector[j])
                    {
                        ++ii;
                        ++j;
                    }
                    else
                    {
                        break;
                    }
                }

                if (j < subvector.size() && ii == history.size()) 
                {
                    int k = 0;
                    for (k = 0; k < view.size() && j + k < subvector.size(); ++k)
                    {
                        if (view[k] != subvector[j + k])
                        {
                            break;
                        }
                    }

                    if (k + j == subvector.size())
                    {
                        return i;
                    }
                }
                else
                {
                    if (j == subvector.size())
                    {
                        return i;
                    }
                }
            }
        }

        return -1;
    }

    int doStep(std::vector<unsigned char>& history, std::vector<unsigned char>& view, std::vector<LZ77Node*>& nodes)
    {
        // std::vector<unsigned char> unitedVector(history.size() + view.size());

        // for (int i = 0; i < history.size(); ++i)
        // {
        //     unitedVector[i] = history[i];
        // }

        // for (int i = 0; i < view.size(); ++i)
        // {
        //     unitedVector[i + history.size()] = view[i];
        // }

        std::vector<unsigned char> vectorToFind;
        int foundIndex = -1;
        int i;
        for (i = 0; i < view.size() - 1; ++i)
        {
            vectorToFind.push_back(view[i]);
            int newFoundIndex = find(history, view, vectorToFind);

            if (newFoundIndex == -1 || newFoundIndex >= history.size())
            {
                break;
            }
            
            foundIndex = newFoundIndex;
        }

        if (foundIndex == -1)
        {
            nodes.push_back(new LZ77Node(0, 0, view[0]));
        }
        else
        {
            nodes.push_back(new LZ77Node(history.size() - foundIndex, i, view[i]));
        }

        return i + 1;
    }

    void writeNodesToFile(std::vector<LZ77Node*>& nodes, bool flushQueue, std::queue<bool>& bits, FileWriter* fileWriter)
    {
        int bitsPerOffset = (int)std::log2(historySize);
        int bitsPerLength = (int)std::log2(viewSize);
        std::vector<unsigned char> bytesToWrite;
        int count = 0;

        for (LZ77Node* node : nodes)
        {
            ++count;
            nn.push_back(node);
            std::vector<bool> tempBits = getBitsFromInt(node->offset, bitsPerOffset);
            for (bool bit : tempBits)
            {
                bits.push(bit);
            }

            tempBits = getBitsFromInt(node->length, bitsPerLength);
            for (bool bit : tempBits)
            {
                bits.push(bit);
            }

            tempBits = getBitsFromByte(node->nextChar);
            for (bool bit : tempBits)
            {
                bits.push(bit);
            }

            bytesToWrite.clear();

            while (bits.size() >= 8)
            {
                bytesToWrite.push_back(getByteFromQueue(&bits));
            }

            fileWriter->Write(&bytesToWrite);
        }

        if (flushQueue)
        {
            unsigned char lastByteSize = bits.size();
            unsigned char lastByte = 0;

            for (int i = 7; i >= 0; --i)
            {
                if (bits.empty())
                {
                    break;
                }

                if (bits.front())
                {
                    lastByte += (unsigned char)pow(2, i);
                }

                bits.pop();
            }

            bytesToWrite.clear();
            bytesToWrite.push_back(lastByte);
            bytesToWrite.push_back(lastByteSize);

            fileWriter->Write(&bytesToWrite);
        }
    }

    
    void getLZ77Node(std::vector<bool>& bits, LZ77Node* node)
    {
        int offset = getIntFromBits(bits, (int)log2(historySize));
        int length = getIntFromBits(bits, (int)log2(viewSize));
        unsigned char byte = (unsigned char)getIntFromBits(bits, 8);

        if (offset == 0 && length == 0) 
        {
            node->offset = 0;
            node->length = 0;
        }
        else
        {
            node->offset = (offset == 0 ? historySize : offset);
            node->length = (length == 0 ? viewSize : length);
        }

        node->nextChar = byte;
    }

public:
    LZ77Archiver(int historySize, int viewSize)
    {
        this->historySize = historySize;
        this->viewSize = viewSize;
    }

    void Archive(const std::string inputFile, const std::string outFile) override
    {
        std::vector<unsigned char> history;
        std::vector<unsigned char> view;
        std::vector<LZ77Node*> nodes;

        int readBytes;
        FileReader* fileReader = new FileReader(inputFile);

        for (int i = 0; i < viewSize; ++i)
        {
            unsigned char byte = fileReader->ReadNextChar(readBytes);

            if (readBytes == 0)
            {
                break;
            }

            view.push_back(byte);
        }

        do
        {
            int foundPrefixLength = doStep(history, view, nodes);

            for (int i = 0; i < foundPrefixLength; ++i)
            {
                if (history.size() == historySize)
                {
                    history.erase(history.begin());
                }

                history.push_back(view.front());
                view.erase(view.begin());

                unsigned char byte = fileReader->ReadNextChar(readBytes);

                if (readBytes != 0)
                {
                    view.push_back(byte);
                }
            }
        }
        while (view.size() > 0);

        std::queue<bool> bits;
        FileWriter* fileWriter = new FileWriter(outFile);
        writeNodesToFile(nodes, true, bits, fileWriter);

        delete fileReader;
        delete fileWriter;
    }

    // void DecodeNodes(std::vector<LZ77Node*>& nodes)
    // {
    //     std::vector<unsigned char> decodedBytes;

    //     for (int i = 0; i < nodes.size(); ++i)
    //     {
    //         if (nodes[i]->length == 0 && nodes[i]->offset == 0)
    //         {
    //             decodedBytes.push_back(nodes[i]->nextChar);
    //         }
    //         else
    //         {
    //             int a = decodedBytes.size() - (nodes[i]->offset == 0 ? historySize : nodes[i]->offset);

    //             for (int j = a; j < a + nodes[i]->length; ++j)
    //             {
    //                 decodedBytes.push_back(decodedBytes[j]);
    //             }

    //             decodedBytes.push_back(nodes[i]->nextChar);
    //         }
    //     }

    //     FileWriter* fw = new FileWriter("asdasdsd");
    //     fw->Write(&decodedBytes);

    //     delete fw;
    // }

    void Dearchive(const std::string inputFile, const std::string outFile) override
    {
        FileReader* fileReader = new FileReader(inputFile);
        FileWriter* fileWriter = new FileWriter(outFile);

        int readBytes;
        int oneTripleSize = (int)log2(historySize) + (int)log2(viewSize) + 8;
        std::vector<unsigned char> bytes(oneTripleSize);
        std::vector<unsigned char> decodedBytes;
        LZ77Node* node = new LZ77Node();
        int count = 0;

        do
        {
            readBytes = fileReader->Read(&bytes, oneTripleSize);
            std::vector<bool> bits = getBitsFromBytes(bytes, readBytes);
            size_t bitsSize = bits.size();
            while (bits.size() >= oneTripleSize)
            {
                getLZ77Node(bits, node);
                LZ77Node* nnn = nn[count++];

                if (node->length == 0 && node->offset == 0)
                {
                    decodedBytes.push_back(node->nextChar);
                }
                else
                {
                    int a = decodedBytes.size() - node->offset;

                    for (int i = a; i < a + node->length; ++i)
                    {
                        decodedBytes.push_back(decodedBytes[i]);
                    }

                    decodedBytes.push_back(node->nextChar);
                }
            }
        }
        while (readBytes == oneTripleSize);

        int sz = decodedBytes.size(); 
        fileWriter->Write(&decodedBytes);

        delete node;
        delete fileWriter;
        delete fileReader;
    }

    std::string GetDescription() override 
    {
        return "LZ77";
    }

    std::string GetShortName() override
    {
        return "lz77" + std::to_string(((historySize + viewSize) / 1024));
    }
};