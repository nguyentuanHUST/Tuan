#include "Protocol.h"
#include <iostream>
#include <fstream>
#include <bitset>
#include <cstring>
Message::Message()
{
}
Message::Message(Header mHeader, const uint8_t* pBuf)
{
    setHeader(mHeader);
    setData(pBuf);
}
Message::~Message()
{
}
void Message::setHeader(const Header header)
{
    mHeader = header;
}
Header& Message::getHeader()
{
    return mHeader;
}
void Message::setData(const uint8_t* pBuf)
{
    data.assign(pBuf, pBuf + mHeader.dataLength);
}
uint8_t Message::calBCC()
{
    uint8_t* pHeader = (uint8_t*)&mHeader;
    uint8_t temp;
    for(int i = 0; i<24; i++ ) {
        temp ^= *(pHeader + i);
    }
    for(auto &i:data) {
        temp ^= i;
    }
    mBCC = temp;
    return temp;
}
void Message::setBCC(const uint8_t bcc)
{
    mBCC = bcc;
}
bool Message::verify()
{
    return mBCC == calBCC();
}
void Message::display()
{
    std::cout << "Start character: " << mHeader.startCharacter[0] << mHeader.startCharacter[1] << std::endl;
    std::cout << "Command Mark: " << std::hex <<(uint16_t)mHeader.commandMark << std::endl;
    std::cout << "Response Sign: " << std::hex << (uint16_t)mHeader.responseSign << std::endl;
    std::cout << "VIN: " << mHeader.vin << std::endl;
    std::cout << "Encrypt type: "<< std::hex << (uint16_t)mHeader.encrypType << std::endl;
    std::cout << "Data length: " << mHeader.dataLength <<std::endl;
    std::cout << "Data: ";
    if(mHeader.commandMark == 0x02 | mHeader.commandMark == 0x03) {
        for(auto &i:data) {
            std::cout << i;
        }
    }
    std::cout << std::endl << "BCC: " << std::bitset<8>(mBCC)<<std::endl;
}
void Message::print(const char* path)
{
    std::ofstream out(path, std::ios_base::app);
    out << "Start character: " << mHeader.startCharacter[0] << mHeader.startCharacter[1] << std::endl;
    out << "Command Mark: " << std::hex << (uint16_t)mHeader.commandMark << std::endl;
    out << "Response Sign: " << std::hex << (uint16_t)mHeader.responseSign << std::endl;
    out << "VIN: " <<mHeader.vin << std::endl;
    out << "Encrypt type: "<<std::hex<<(uint16_t)mHeader.encrypType << std::endl;
    out << "Data length: " << std::hex << mHeader.dataLength <<std::endl;
    out << "Data: ";
    if(mHeader.commandMark == 0x02 | mHeader.commandMark == 0x03) {
        for(auto &i:data) {
            std::cout << i;
        }
    }
    out << std::endl << "BCC: " << std::bitset<8>(mBCC)<<std::endl;
}
std::unique_ptr<uint8_t[]> Message::deserialize() {
    const uint16_t headerLen = sizeof(Header);
    const uint8_t bccLen = 1;
    std::unique_ptr<uint8_t[]> p = std::make_unique<uint8_t[]>(headerLen + mHeader.dataLength + bccLen);
    uint8_t* delegateP = p.get();
    std::memcpy(delegateP, &mHeader, headerLen);
    delegateP += headerLen;
    std::memcpy(delegateP, &data[0], mHeader.dataLength);
    delegateP += mHeader.dataLength;
    std::memcpy(delegateP, &mBCC, bccLen);
    return p;
}

uint32_t Message::getMessageLength() {
    return sizeof(Header) + mHeader.dataLength + 1;
}
