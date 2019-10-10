#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <boost/bind.hpp>
#include <boost/date_time.hpp>
#include <fstream>
#include <Protocol.h>
#include "ISocket.h"
using namespace boost::asio;
using namespace boost::posix_time;
class Server
{
    public:
    Server();
    void ack();
    void start();
    void onAccept(const boost::system::error_code& error);
    void runIO();
    void stop();
    private:
    static const int max_size = 65000;
    io_service mService;
    ip::tcp::acceptor mAcceptor;
    std::unique_ptr<uint8_t[]> pData;
    std::unique_ptr<uint8_t[]> pBufReceive;
    std::vector<std::unique_ptr<ISocket>> mSockets;
    uint16_t nPacket{0};
};