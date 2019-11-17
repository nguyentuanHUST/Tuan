#pragma once
#include "ISocket.h"
#include "ICommHandler.h"
#include "boost/asio/ssl.hpp"
using namespace boost::asio;
typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;
class SSLSocket:public ISocket
{
public:
    SSLSocket(io_service& service, boost::asio::ssl::context& context);
    ~SSLSocket();
    void async_receive(uint32_t len = maxsize);
    void async_send(boost::asio::mutable_buffer mData);
    uint32_t receive(int32_t len);
    uint32_t send(boost::asio::mutable_buffer mData);
    boost::asio::ip::tcp::socket& getSocket();
    void setCommHandler(ICommHandler* const ptr);
    void setID(int id);
private:
    void onReceive(boost::system::error_code error, std::size_t bytes_transferred);
    void onSend(boost::system::error_code error, std::size_t bytes_transferred);
    void onReadUntil(const boost::system::error_code& e, std::size_t size);
    void onHandShake(boost::system::error_code error);
private:
    static const uint32_t maxsize = 65535;
    io_service& mService;
    ssl_socket mSocket;
    std::unique_ptr<uint8_t[]> pBufSend;
    std::unique_ptr<uint8_t[]> pBufReceive;
    uint64_t size;
    std::shared_ptr<ICommHandler> mCommHandler;
    int id;
    boost::asio::streambuf pStreamBuf;
    // boost::asio::ssl::context& mContext;
};

