#include <iostream>
#include <asio.hpp>

using asio::ip::tcp;

int main() {
    std::cout << "M4CHAT" << std::endl;

    try {

    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

class tcp_server {
public:
    tcp_server(asio::io_context& io_context) : io_context_(io_context),
    acceptor_(io_context, tcp::endpoint(tcp::v4(), 13)) {
        start_accepting();
    }


private:
    asio::io_context& io_context_;
    tcp::acceptor acceptor_;

    void start_accepting() {
        
    }
};