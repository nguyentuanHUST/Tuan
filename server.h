#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <boost/bind.hpp>
#include <boost/date_time.hpp>
#include <Protocol.h>
#include "ISocket.h"
#include "ICommHandler.h"
#include <mutex>
#include <map>
#include <boost/asio/ssl.hpp>
using namespace boost::asio;
using namespace boost::posix_time;
class Server
{
    public:
    Server();
    void ack();
    void start();
    void onAccept(const boost::system::error_code& error);
    void onSSLAccept(const boost::system::error_code& error);
    void runIO();
    void stop();
    void setMode(int mode);
    private:
    class myCommHandler : public ICommHandler
    {
        public:
        myCommHandler(Server& app): mApp(app) {}
        void onReceive(uint8_t* pData, uint32_t len, int id) override;
        void onSend(bool result, int id) override;
        void onDisconnected(int id) override;
        private:
        Server& mApp;
    };
    static const int max_size = 65000;
    static int socketID;
    int TCPAcceptorID;
    int TLSAcceptorID;
    io_service mService;
    boost::asio::ssl::context mContext;
    ip::tcp::acceptor mAcceptor;
    ip::tcp::acceptor mSSLAcceptor;
    std::unique_ptr<uint8_t[]> pData;
    std::unique_ptr<uint8_t[]> pBufReceive;
    std::map<int, std::unique_ptr<ISocket>> mSockets;
    uint16_t nPacket{0};
    std::shared_ptr<myCommHandler> mCommHandler;
    std::mutex lockHandler;
    int mode;
    struct Response {
        int id;
        Message msg;
    } response;
};