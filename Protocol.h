#pragma once
#include <cstdint>
#include <vector>
#include <memory>

typedef struct Header
{
    unsigned char startCharacter[2];
    uint8_t commandMark;
    uint8_t responseSign;
    unsigned char vin[17];
    uint8_t encrypType;
    uint16_t dataLength;
} Header;
class Message
{
    private:
    Header mHeader;
    std::vector<uint8_t> data;
    uint8_t mBCC;
    public:
    Message();
    Message(Header mHeader, const uint8_t* pBuf);
    ~Message();
    void setHeader(const Header header);
    Header& getHeader();
    void setData(const uint8_t* pBuf);
    uint8_t calBCC();
    void setBCC(const uint8_t bcc);
    bool verify();
    void display();
    void print(const char* path);
    std::unique_ptr<uint8_t[]> deserialize();
    uint32_t getMessageLength();
    void serialize(uint8_t* pData, uint32_t len);
};