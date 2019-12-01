#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include "Protocol.h"
#include <stdio.h>
#include <boost/asio/ssl.hpp>
using namespace boost::asio;
using namespace boost::posix_time;
typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

class Client
{
    public:
    Client():mContext(boost::asio::ssl::context::sslv23),
    mSocket(mService, mContext){
        pData = std::make_unique<uint8_t[]>(max_size);
        pBufReceive = std::make_unique<uint8_t[]>(max_size);
    }
    void onReceive(boost::system::error_code error, std::size_t bytes_transferred) 
    {
        std::cout << "Byte received: "<<bytes_transferred<<std::endl;
        if(!error){
            Message msg;
            msg.serialize(pBufReceive.get(), bytes_transferred);
            msg.display();
            if(msg.verify()) {
                std::cout  << "Verify response success" << std::endl;
            } else {
                std::cout  << "Verify fail" << std::endl;
            }
            clientReceive();
        } else {
            std::cout<<"Error "<<error.message()<<std::endl;
        }
    }
    void onReadUntil(boost::system::error_code error, std::size_t bytes_transferred) 
    {
        if(!error){
            async_read_until(mSocket, buffer, '\0',boost::bind(&Client::onReadUntil,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
        } else {
            std::cout<<"Error "<<error.message()<<std::endl;
        }
    }
    void onSend(boost::system::error_code error, std::size_t bytes_transferred) 
    {
        cbTime = microsec_clock::local_time();
        std::cout<<"Timediff send: "<< (cbTime - reqTime).total_milliseconds()<<std::endl;
        if(!error){
            std::cout<<"Send " << bytes_transferred << std::endl;
            // boost::this_thread::sleep(boost::posix_time::millisec(1000));
            // sleep(1);
            // clientSend();
        } else {
            std::cout<<"Error "<<error.message()<<std::endl;
        }
    }
    void start()
    {
        std::cout<<"Client start\n";
        mContext.load_verify_file("certificate.pem");
        mSocket.set_verify_mode(boost::asio::ssl::verify_peer);
        mSocket.set_verify_callback(boost::bind(&Client::verify_certificate, this, _1, _2));
        // ::ip::tcp::endpoint ep(::ip::address::from_string("127.0.0.1"), 8891);
        // mSocket.async_connect();
        boost::asio::ip::tcp::resolver resolver(mService);
        boost::asio::ip::tcp::resolver::query query("localhost", "8891");
        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
        boost::asio::async_connect(mSocket.lowest_layer(), iterator, [this](...) {
            std::cout<<"Connected\n";
            this->mSocket.async_handshake(boost::asio::ssl::stream_base::client, [this](const boost::system::error_code& error){
                if (!error)
                {
                    this->clientReceive();
                }
                else
                {
                    std::cout << "Handshake failed: " << error.message() << "\n";
                }
            });
        });
    }
bool verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx)
  {
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once
    // for each certificate in the certificate chain, starting from the root
    // certificate authority.

    // In this example we will simply print the certificate's subject name.
    char subject_name[256];
    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    std::cout << "Verifying " << subject_name << "\n";

    return preverified;
  }
    void send(const char* data, const uint16_t length)
    {
        mSocket.async_write_some(boost::asio::buffer(data,length), boost::bind(&Client::onSend,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
        reqTime = microsec_clock::local_time();
        std::cout<< "Send timestamp(ms): "<<reqTime.time_of_day().total_milliseconds()<<std::endl;
    }

    void startIO()
    {
        mService.run();
    }
    private:
    void clientSend()
    {
        mSocket.async_write_some(boost::asio::buffer(pData.get(),max_size), boost::bind(&Client::onSend,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
        reqTime = microsec_clock::local_time();
        std::cout<< "Send timestamp(ms): "<<reqTime.time_of_day().total_milliseconds()<<std::endl;
    }
    void clientReceive(const uint64_t size = Client::max_size)
    {
        mSocket.async_read_some(boost::asio::buffer(pBufReceive.get(),max_size), boost::bind(&Client::onReceive,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
    }
    static const uint64_t max_size = 65600;
    io_service mService;
    boost::asio::ssl::context mContext;
    ssl_socket mSocket;
    std::unique_ptr<uint8_t[]> pData;
    uint16_t size;
    std::unique_ptr<uint8_t[]> pBufReceive;
    ptime reqTime;
    ptime cbTime;
    uint64_t nPacket{0};
    boost::asio::streambuf buffer;
};
int main(int argc, char** argv)
{
    Client client;
    client.start();
    Header header;//{"##", 0x01, 0xFE, "0123456789ABCDE", 0x01, 0};
    header.startCharacter[0] = '#';
    header.startCharacter[1] = '#';
    header.commandMark = 0x01;
    header.responseSign = 0xFE;
    const char* vin = "0123456789ABCDEF";
    memcpy(header.vin, vin, 17);
    header.encrypType = 0x01;
    header.dataLength = 5;
    Message msg;
    msg.setHeader(header);
    char* c = "abcde";
    msg.setData(reinterpret_cast<uint8_t *>(c));
    (void)msg.calBCC();
    msg.display();
    std::unique_ptr<uint8_t[]> ptr = msg.deserialize();
    char* s = reinterpret_cast<char *>(ptr.get());
    client.send(s, msg.getMessageLength());
    client.send(s, msg.getMessageLength());
    client.send(s, msg.getMessageLength());
    client.startIO();
    return 0;
}