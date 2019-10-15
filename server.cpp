#include "server.h"
#include "Socket.h"

using namespace boost::asio;
using namespace boost::posix_time;

int Server::socketID = 1;
Server::Server():mAcceptor(mService, ip::tcp::endpoint(ip::tcp::v4(), 6969)){
    pData = std::make_unique<uint8_t[]>(100);
    pBufReceive = std::make_unique<uint8_t[]>(max_size);
}
void Server::ack(){

}

void Server::start()
{
    std::cout<<"Start server \n";
    mCommHandler = std::make_shared<myCommHandler>(myCommHandler(*this));
    std::unique_ptr<Socket> pSocket = std::make_unique<Socket>(mService);
    pSocket->setCommHandler(mCommHandler.get());
    pSocket->setID(socketID);
    mAcceptor.async_accept(pSocket->getSocket(), boost::bind(&onAccept, this, placeholders::error));
    mSockets.insert(std::pair<int, std::unique_ptr<ISocket>>(socketID, std::move(pSocket)));
}

void Server::onAccept(const boost::system::error_code& error) 
{
    if(!error){
        auto it = mSockets.find(socketID)->second.get();
        it->async_receive(max_size);
        socketID += 1;
        std::unique_ptr<Socket> pSocket = std::make_unique<Socket>(mService);
        pSocket->setCommHandler(mCommHandler.get());
        pSocket->setID(socketID);
        mAcceptor.async_accept(pSocket->getSocket(), boost::bind(&onAccept, this, placeholders::error));
        mSockets.insert(std::pair<int, std::unique_ptr<ISocket>>(socketID, std::move(pSocket)));
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

void Server::myCommHandler::onReceive(uint8_t* pData, uint32_t len, int id)
{
    std::lock_guard<std::mutex> lock(mApp.lockHandler);
    ISocket* socket = mApp.mSockets.find(id)->second.get();
    Header header;
    try{
        std::memcpy(&header, pData, sizeof(Header));
        Message msg{header, pData + sizeof(Header)};
        msg.setBCC(*(pData + sizeof(Header) + header.dataLength));
        msg.display();
        if(msg.verify()) {
            switch(msg.getHeader().commandMark) {
                case 0x01: 
                    std::cout << "0x01 msg" << std::endl;
                    msg.getHeader().responseSign = 0x01;
                    msg.calBCC();
                    {
                    std::unique_ptr<uint8_t[]> ptr = msg.deserialize();
                    char* s = reinterpret_cast<char *>(ptr.get());
                    socket->async_send(boost::asio::mutable_buffer(s, msg.getMessageLength()));
                    std::cout << msg.getMessageLength() << sizeof(Header) << std::endl;
                    msg.display();
                    }
                    break;
                case 0x04: 
                    std::cout << "0x04 msg" << std::endl;
                    break;
                case 0x07: 
                    std::cout << "0x07 msg" << std::endl;
                    break;
                default:
                    std::cout << "Not identify" << std::endl;
            }
        } else {
            std::cout << "Verify msg failed" << std::endl;
        }
    } catch (std::exception e) {
        std::cout << e.what() << std::endl;
    }
    
}
void Server::myCommHandler::onSend(bool result, int id)
{
    std::lock_guard<std::mutex> lock(mApp.lockHandler);
}
void Server::myCommHandler::onDisconnected(int id)
{

}
int main(int argc, char** argv)
{
    Server server;
    server.start();
    server.runIO();
    return 0;
}