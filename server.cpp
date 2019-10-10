#include "server.h"
#include "Socket.h"

using namespace boost::asio;
using namespace boost::posix_time;
Server::Server():mAcceptor(mService, ip::tcp::endpoint(ip::tcp::v4(), 6969)){
    pData = std::make_unique<uint8_t[]>(100);
    pBufReceive = std::make_unique<uint8_t[]>(max_size);
}
void Server::ack(){

}

void Server::start()
{
    std::cout<<"Start server \n";
    std::unique_ptr<Socket> pSocket = std::make_unique<Socket>(mService);
    mAcceptor.async_accept(pSocket->getSocket(), boost::bind(&onAccept, this, placeholders::error));
    mSockets.push_back(std::move(pSocket));
}

void Server::onAccept(const boost::system::error_code& error) 
{
    if(!error){
        std::cout<<"Accepted\n";
        auto it = mSockets.back().get();
        it->async_receive(max_size);
        std::unique_ptr<Socket> pSocket = std::make_unique<Socket>(mService);
        mAcceptor.async_accept(pSocket->getSocket(), boost::bind(&onAccept, this, placeholders::error));
        mSockets.push_back(std::move(pSocket));
    } else {
        std::cout<<error.message()<<std::endl;
    }
}
void Server::runIO()
{
    mService.run();
}
void Server::stop()
{
    std::cout<<"Server stop\n";
}

int main(int argc, char** argv)
{
    Server server;
    server.start();
    server.runIO();
    return 0;
}