#pragma once
#include <memory>
#include <cstdint>
#include "ISocket.h"
class ICommHandler
{
public:
    virtual ~ICommHandler(){}
    virtual void onReceive(uint8_t* pData, uint32_t len, int id) = 0;
    virtual void onSend(bool result, int id) = 0;
    virtual void onDisconnected(int id) = 0;
};