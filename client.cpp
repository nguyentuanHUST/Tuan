#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
using namespace boost::asio;
using namespace boost::posix_time;


class Client
{
    public:
    Client(){
        std::cout<<"Constructor\n";
        pData = std::make_unique<uint8_t[]>(max_size);
        pBufReceive = std::make_unique<uint8_t[]>(max_size);
    }
    void onReceive(boost::system::error_code error, std::size_t bytes_transferred) 
    {
        if(!error){
            std::cout<< pBufReceive.get() << " ack length: " << bytes_transferred<< std::endl;
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
            sleep(1);
            clientSend();
        } else {
            std::cout<<"Error "<<error.message()<<std::endl;
        }
    }
    void start()
    {
        std::cout<<"Client start\n";
        ::ip::tcp::endpoint ep(::ip::address::from_string("127.0.0.1"), 6969);
        mSocket.async_connect(ep, [this](...) {
            std::cout<<"Connected\n";
            clientSend();
            clientReceive();
        });
        mService.run();
    }

    void send(const uint8_t* data, const uint16_t length)
    {

    }

    private:
    void clientSend()
    {
        mSocket.async_send(boost::asio::buffer(pData.get(),max_size), boost::bind(&Client::onSend,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
        reqTime = microsec_clock::local_time();
        std::cout<< "Send timestamp(ms): "<<reqTime.time_of_day().total_milliseconds()<<std::endl;
    }
    void clientReceive(const uint64_t size = Client::max_size)
    {
        mSocket.async_receive(boost::asio::buffer(pBufReceive.get(),20), boost::bind(&Client::onReceive,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
    }
    static const uint64_t max_size = 100*1024*1024;
    io_service mService;
    ip::tcp::socket mSocket{mService};
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
    while(1){}
    return 0;
}