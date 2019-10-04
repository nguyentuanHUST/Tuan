#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <boost/bind.hpp>
#include <boost/date_time.hpp>
#include <fstream>
#include <Protocol.h>
using namespace boost::asio;
using namespace boost::posix_time;
class Server
{
    public:
    Server():mSocket(mService), mAcceptor(mService, ip::tcp::endpoint(ip::tcp::v4(), 6969)){
        pData = std::make_unique<uint8_t[]>(100);
        pBufReceive = std::make_unique<uint8_t[]>(max_size);
    }
    void onReceive(boost::system::error_code error, std::size_t bytes_transferred) 
    {
        ptime recvTime = microsec_clock::local_time();
        std::cout<< " Receive timestamp(ms): "<<recvTime.time_of_day().total_milliseconds()<<std::endl;
        if(!error){
            std::cout<<"Received " << bytes_transferred << std::endl;
            mSocket.async_receive(boost::asio::buffer(pBufReceive.get(),max_size), boost::bind(&onReceive,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
            if(nPacket < 5000) {
                ack();
            }
        } else {
            std::cout<<"Error "<<error.message()<<std::endl;
        }
    }
    void ack(){
        nPacket++;
        std::string ack = "Received success " + std::to_string(nPacket) + " #";
        memcpy(pData.get(), ack.c_str(), ack.length());
        mSocket.async_send(boost::asio::buffer(pData.get(),ack.length()), boost::bind(&onSend,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
        ptime sendTime = microsec_clock::local_time();
        std::cout<< ack <<" timestamp(ms): "<<sendTime.time_of_day().total_milliseconds()<<std::endl;
    }
    void onSend(boost::system::error_code error, std::size_t bytes_transferred) 
    {
        if(!error){
            //std::cout<<"Send " << bytes_transferred << std::endl;
        } else {
            std::cout<<"Error "<<error.message()<<std::endl;
        }
    }

    void start()
    {
        std::cout<<"Start server \n";
        mAcceptor.async_accept(mSocket, [this](...) {
            std::cout<<"Accepted\n";
            mSocket.async_receive(boost::asio::buffer(pBufReceive.get(),max_size), boost::bind(&onReceive,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
        });
              
        
    }
    void runIO()
    {
        mService.run();
    }
    void stop()
    {
        std::cout<<"Server stop\n";
    }
    private:
    static const int max_size = 100*1024*1024;
    io_service mService;
    ip::tcp::socket mSocket;
    ip::tcp::acceptor mAcceptor;
    std::unique_ptr<uint8_t[]> pData;
    std::unique_ptr<uint8_t[]> pBufReceive;
    ip::tcp::endpoint ep;
    uint16_t nPacket{0};
};
int main(int argc, char** argv)
{
    // io_service service;
    // ip::tcp::endpoint ep( ip::tcp::v4(), 6969); // listen on 2001
    // ip::tcp::acceptor acc(service, ep);
    // ip::tcp::socket mSocket{service};
    // acc.async_accept(mSocket,[](...){
    //     std::cout<<"accepted\n";
    // });
    // service.run();
    Server server;
    server.start();
    server.runIO();
    return 0;
}