// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// My code is based on Christopher M. Kohlhoff's chat client example code.

#include <deque>
#include <iostream>
#include <fstream>
#include <thread>
#include "asio.hpp"
#include "../include/chat_message.h"

using asio::ip::tcp;

typedef std::deque<chat_message> chat_message_queue;

class chat_client
{
public:
    chat_client(asio::io_context& io_context,
                const tcp::resolver::results_type& endpoints)
            : io_context_(io_context),
              socket_(io_context)
    {
        do_connect(endpoints);
    }

    void write(const chat_message& msg)
    {
        asio::post(io_context_,
                   [this, msg]()
                   {
                       bool write_in_progress = !write_msgs_.empty();
                       write_msgs_.push_back(msg);
                       if (!write_in_progress)
                       {
                           do_write();
                       }
                   });
    }

    void close()
    {
        asio::post(io_context_, [this]() { socket_.close(); });
    }

private:
    void do_connect(const tcp::resolver::results_type& endpoints)
    {
        asio::async_connect(socket_, endpoints,
                            [this](std::error_code ec, const tcp::endpoint&)
                            {
                                if (!ec) do_read_header();

                                else {
                                std::cout << "error" << std::endl;
                                throw std::runtime_error("rc" + std::to_string(ec.value()) +
                                "Failed to connect to a server, make sure that there is a server hosted on the given domain.");
                                }
                            });
    }

    void do_read_header()
    {
        std::cout << "Reading headers :>" << std::endl;

        asio::async_read(socket_,
                         asio::buffer(read_msg_.data(), chat_message::header_length),
                         [this](std::error_code ec, std::size_t /*length*/)
                         {
                             if (!ec && read_msg_.decode_header())
                             {
                                 do_read_body();
                             }
                             else
                             {
                                 socket_.close();
                             }
                         });
    }

    void do_read_body()
    {
        asio::async_read(socket_,
                         asio::buffer(read_msg_.body(), read_msg_.body_length()),
                         [this](std::error_code ec, std::size_t /*length*/)
                         {
                             if (!ec)
                             {
                                 std::cout.write(read_msg_.body(), static_cast<int>(read_msg_.body_length()));
                                 std::cout << "\n";
                                 do_read_header();
                             }
                             else
                             {
                                 socket_.close();
                             }
                         });
    }

    void do_write()
    {
        asio::async_write(socket_,
                          asio::buffer(write_msgs_.front().data(),
                                       write_msgs_.front().length()),
                          [this](std::error_code ec, std::size_t /*length*/)
                          {
                              if (!ec)
                              {
                                  write_msgs_.pop_front();
                                  if (!write_msgs_.empty())
                                  {
                                      do_write();
                                  }
                              }
                              else
                              {
                                  socket_.close();
                              }
                          });
    }

private:
    asio::io_context& io_context_;
    tcp::socket socket_;
    chat_message read_msg_;
    chat_message_queue write_msgs_;
};

struct config_values {
    std::string username;
    std::string res_ip;
//    unsigned short int ac_port;
    std::string res_port;
};

//Config format is something as following: nickname="matexpl" domain="localhost:8088"
//Returns an empty string if the word is not found
std::string extract_config_value(const std::string& keyword, const std::string& text) {
    size_t nick_pos;

    if ((nick_pos = text.find(keyword)) == std::string::npos) return "";

    size_t value_search_pos = text.find('=', nick_pos);
    size_t value_start = text.find('"', value_search_pos) + 1; // Add one to skip over the quotation character

    return text.substr(value_start, text.find('"', value_start) - value_start);
}

bool read_config(config_values& config) {
    std::string line;
    std::ifstream config_file("config");

    if (!config_file.is_open()) return false;

    while(std::getline(config_file, line)) {
        if (line.find('#') != std::string::npos) continue;

        std::string result;

        if (!(result = extract_config_value("username", line)).empty()) {
            config.username = result;
        }

        if (!(result = extract_config_value("domain", line)).empty()) {
            size_t separator;
            if((separator = result.find(':')) == std::string::npos) {
                std::cout << "[Error]: Wrong domain config format, use the following format: \"<server_ip>:<server_port>\"" << std::endl;
                return false;
            }

            config.res_ip = result.substr(0, separator);
            config.res_port = result.substr(separator + 1);
        }
    }

    if (config.username.empty()) {
        std::cout << "[Error]: No username specified in the config file. Please edit the configuration file in a text editor and assign a username" << std::endl;

        return false;
    }

    return true;
}

/* ==== Program return codes ====
 * 0 = terminated successfully
 * 1 = configuration file error
 * 2 = failed to connect or resolve
 * 3 = internal error, not specified by custom code
 * 101 = something went really wrong */
int main() {
    try {
        std::cout << "-------------------------------------------\n";
        std::cout << "                   m4chat                  \n";
        std::cout << "-------------------------------------------\n";

        config_values config;

        if(!read_config(config)) {
            std::cout << "[Error]: Configuration file error. Make sure the config "
                         "file is within the program directory and is formatted properly." << std::endl;

            std::cout << "Program ended, press enter to close." << std::endl;
            std::cin.get();

            return 1;
        }

        if (config.res_ip.empty()) {
            std::cout << "Server IP (localhost): ";
            std::getline(std::cin, config.res_ip);

            std::cout << "Server Port (8088): ";
            std::getline(std::cin, config.res_port);
        }

        if (config.res_ip.empty()) config.res_ip = "localhost";
        if (config.res_port.empty()) config.res_port = "8088";

        asio::io_context io_context;

        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(config.res_ip, config.res_port);

        // Compare to an empty iterator to check whether it's empty
        if (endpoints == tcp::resolver::iterator()) {
            throw std::runtime_error("rc2Resolved no endpoints. Make sure the domain provided is correct.");
        }

        std::cout << "Resolved " << config.res_ip << ":" << config.res_port << std::endl;

        chat_client c(io_context, endpoints);

        // Starts a thread for the io_context event loop
        std::thread t([&io_context]() { io_context.run(); });

        std::cout << "==========================\n" <<
                     "   Chat session started   \n" <<
                     "==========================\n" <<
                     "     Last 10 messages     \n" << std::endl;


        char line[chat_message::max_body_length + 1];
        while (std::cin.getline(line, chat_message::max_body_length + 1)) {
            if (line[0] == '\0') continue;

            chat_message msg;
            msg.body_length(std::strlen(line));
            std::memcpy(msg.body(), line, msg.body_length());
            msg.encode_header();
            c.write(msg);
        }

        c.close();
        t.join();
    }

    // Errors starting with "rc" are errors with a return code where the code is the number proceeding it.
    catch (std::exception &e) {
        std::string content = e.what();
        unsigned short int code = 3;

        //Works for single digit codes, good enough for now
        if (content[0] == 'r' && content[1] == 'c') {
            code = content[2] - '0'; //Character to digit conversion
            content.erase(0, 3);
        }

        std::cerr << "[Error]: " << content << std::endl;

        // Instead of flushing. Even when I flush both cout and cerr
        // somehow the error message comes after the press enter message...
        sleep(1);

        std::cout << "Press enter to close." << std::endl;
        std::cin.get();

        return code;
    }

    return 101;
}

//TODO cursor thing, state of message sending
//TODO config file, sender nickname
//TODO RSA encryption
//TODO signatures