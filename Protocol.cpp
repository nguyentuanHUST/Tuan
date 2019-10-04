#include "Protocol.h"
#include <iostream>
#include <fstream>
#include <bitset>
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
Header Message::getHeader()
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
    std::cout << "Start character: " << mHeader.startCharacter << std::endl;
    std::cout << "Command Mark: " << std::hex << mHeader.commandMark << std::endl;
    std::cout << "Response Sign: " << std::hex << mHeader.responseSign << std::endl;
    std::cout << "VIN: " <<mHeader.vin << std::endl;
    std::cout << "Encrypt type: "<<std::hex<<mHeader.encrypType << std::endl;
    std::cout << "Data length: " << std::hex << mHeader.dataLength <<std::endl;
    std::cout << "Data: ";
    for(auto &i:data) {
        std::cout << i;
    }
    std::cout << std::endl << "BCC: " << std::bitset<8>(mBCC)<<std::endl;
}
void Message::print(const char* path)
{
    std::ofstream out(path);
    out << "Start character: " << mHeader.startCharacter << std::endl;
    out << "Command Mark: " << std::hex << mHeader.commandMark << std::endl;
    out << "Response Sign: " << std::hex << mHeader.responseSign << std::endl;
    out << "VIN: " <<mHeader.vin << std::endl;
    out << "Encrypt type: "<<std::hex<<mHeader.encrypType << std::endl;
    out << "Data length: " << std::hex << mHeader.dataLength <<std::endl;
    out << "Data: ";
    for(auto &i:data) {
        out << i;
    }
    out << std::endl << "BCC: " << std::bitset<8>(mBCC)<<std::endl;
}