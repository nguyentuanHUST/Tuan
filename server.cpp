#include "server.h"
#include "Socket.h"
#include <stdio.h>
#include <sys/select.h>
#include <termios.h>
#include <future>
#include <thread>
#include <chrono>
#include "SSLSocket.h"
// #include <stropts.h>

int _kbhit() {
    static const int STDIN = 0;
    static bool initialized = false;

    if (! initialized) {
        // Use termios to turn off line buffering
        termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}

using namespace boost::asio;
using namespace boost::posix_time;

int Server::socketID = 1;
Server::Server():mAcceptor(mService, ip::tcp::endpoint(ip::tcp::v4(), 8890)),
mSSLAcceptor(mService, ip::tcp::endpoint(ip::tcp::v4(), 8891)),
mContext(boost::asio::ssl::context::sslv23) {
    pData = std::make_unique<uint8_t[]>(100);
    pBufReceive = std::make_unique<uint8_t[]>(max_size);
    mode = 0;
    mContext.set_options(
    boost::asio::ssl::context::default_workarounds
    | boost::asio::ssl::context::no_sslv2
    | boost::asio::ssl::context::single_dh_use);
    // mContext.set_password_callback(boost::bind(&server::get_password, this));
    mContext.use_certificate_chain_file("certificate.pem");
    mContext.use_private_key_file("key.pem", boost::asio::ssl::context::pem);
    // mContext.use_tmp_dh_file("dh512.pem");
}
void Server::ack(){
    std::cout << "Respond client id: " << response.id<<std::endl;
    response.msg.print("Send.txt");
    mSockets.find(response.id) ->second->async_send(boost::asio::buffer(response.msg.deserialize().get(), response.msg.getMessageLength()));
}

void Server::start()
{
    std::cout<<"Start server \n";
    mCommHandler = std::make_shared<myCommHandler>(myCommHandler(*this));
    std::unique_ptr<Socket> pSocket = std::make_unique<Socket>(mService);
    pSocket->setCommHandler(mCommHandler.get());
    pSocket->setID(socketID);
    TCPAcceptorID = socketID;
    mAcceptor.async_accept(pSocket->getSocket(), boost::bind(&onAccept, this, placeholders::error));
    mSockets.insert(std::pair<int, std::unique_ptr<ISocket>>(socketID, std::move(pSocket)));
    socketID += 1;
    std::unique_ptr<SSLSocket> pSSLSocket = std::make_unique<SSLSocket>(mService, mContext);
    pSSLSocket->setCommHandler(mCommHandler.get());
    pSSLSocket->setID(socketID);
    TLSAcceptorID = socketID;
    mSSLAcceptor.async_accept(pSSLSocket->getSocket(), boost::bind(&onSSLAccept, this, placeholders::error));
    mSockets.insert(std::pair<int, std::unique_ptr<ISocket>>(socketID, std::move(pSSLSocket)));

}

void Server::onAccept(const boost::system::error_code& error) 
{
    if(!error){
        std::cout<<"Accepted!\n";
        auto it = mSockets.find(TCPAcceptorID)->second.get();
        it->async_receive(max_size);
        socketID += 1;
        std::unique_ptr<Socket> pSocket = std::make_unique<Socket>(mService);
        pSocket->setCommHandler(mCommHandler.get());
        pSocket->setID(socketID);
        TCPAcceptorID = socketID;
        mAcceptor.async_accept(pSocket->getSocket(), boost::bind(&onAccept, this, placeholders::error));
        mSockets.insert(std::pair<int, std::unique_ptr<ISocket>>(socketID, std::move(pSocket)));
    } else {
        std::cout<<error.message()<<std::endl;
    }
}

void Server::onSSLAccept(const boost::system::error_code& error) 
{
    if(!error){
        auto it = mSockets.find(TLSAcceptorID)->second.get();
        it->async_receive(max_size);
        socketID += 1;
        std::unique_ptr<SSLSocket> pSSLSocket = std::make_unique<SSLSocket>(mService, mContext);
        pSSLSocket->setCommHandler(mCommHandler.get());
        pSSLSocket->setID(socketID);
        TLSAcceptorID = socketID;
        mSSLAcceptor.async_accept(pSSLSocket->getSocket(), boost::bind(&onSSLAccept, this, placeholders::error));
        mSockets.insert(std::pair<int, std::unique_ptr<ISocket>>(socketID, std::move(pSSLSocket)));
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

void Server::setMode(int m)
{
    mode = m;
}

void Server::myCommHandler::onReceive(uint8_t* pData, uint32_t len, int id)
{
    std::lock_guard<std::mutex> lock(mApp.lockHandler);
    ISocket* socket = mApp.mSockets.find(id)->second.get();
    Header header;
    try{
        Message msg;
        msg.serialize(pData, len);
        msg.display();
        msg.print("Receive.txt");
        if(msg.verify()) {
            switch(msg.getHeader().commandMark) {
                case 0x01: 
                    std::cout << "0x01 msg" << std::endl;
                    msg.getHeader().responseSign = 0x01;
                    msg.calBCC();
                    if(mApp.mode == 0) {
                        msg.print("Send.txt");
                        socket->async_send(boost::asio::buffer(msg.deserialize().get(), msg.getMessageLength()));
                    } else if (mApp.mode == 1) {
                        mApp.response = Response{id,msg};
                    }
                    break;
                case 0x02: 
                    std::cout << "0x02 msg" << std::endl;
                    break;
                case 0x03: 
                    std::cout << "0x03 msg" << std::endl;
                    break;
                case 0x04: 
                    std::cout << "0x04 msg" << std::endl;
                    break;
                case 0x07: 
                    std::cout << "0x07 msg" << std::endl;
                    if(mApp.mode == 0) {
                        msg.print("Send.txt");
                        socket->async_send(boost::asio::buffer(msg.deserialize().get(), msg.getMessageLength()));
                    } else if (mApp.mode == 1) {
                        mApp.response = Response{id,msg};
                    }
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
    auto it = mApp.mSockets.find(id);
    if(it != mApp.mSockets.end()) {
        std::cout<<"Session disconnected"<<std::endl;
        // mApp.mSockets.erase(id);        
    } else {
        std::cout<<"Not a valid session"<<std::endl;
    }
}
int main(int argc, char** argv)
{
    Server server;
    std::future<void> f;
    if(argc > 1) {
        server.setMode(std::atoi(argv[1]));
        f = std::async(std::launch::async, [&server](){
            while (true) {
                if(_kbhit()) {
                    char key = getchar();
                    if(key == 's') {
                        server.ack();
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
    }
    server.start();
    server.runIO();
    f.get();
    return 0;
}