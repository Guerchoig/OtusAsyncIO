/**
 * @brief client.cpp
 *        a test client to work with bulk_server
 *        according to '#define AUTO' can be built in one of two modes:
 *        - automatic, when generates a swarm of comands followed by DISCONNECT command
 *        - manual, accepting commands from keyboard, and sending DISCONNECT on CTRL-D
 */
#include "common.h"
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/detail/error_code.hpp>
#include <iostream>

// #define AUTO // If defined enforces coplilation to generate the commands automatically whith disconnect at the end
//  If not defined - client accept commands from keyboard; CTRL-D produce disconnect

namespace asio = boost::asio;

// Client parameters
constexpr int port = 4507;
constexpr auto host = "127.0.0.1";
constexpr int nof_cmds_to_generate = 7;

// Disconnect symbol
constexpr unsigned char DISCONNECT = 0x4;

/**
 * @brief Send string to socket and process errors
 * @param socket
 * @param s string to send
 */
void send_string(asio::ip::tcp::socket &socket, const std::string s)
{
    boost::system::error_code ec;

    auto sent = socket.send(asio::buffer(s), {}, ec);
    assert(!ec);
    assert(sent == s.size());
}

/**
 * @brief If AUTO is defined, accept one parameter - starting order number for generated commands
 *        to mark input from different clients, runnin simultaneously. Commands then are 1 second-interleaved.
 *        if AUTO is undefined, accepts commands from keyboard, CTRL-D then being a 'disconnect from server' command
 * @param argc up to 2
 * @param argv starting order number for generated commands (optional)
 * @return
 */
int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    asio::io_context context;

#ifdef AUTO // Automatic operation

    int start_num = 1;

    if (argc == 2)
        start_num = std::atoi(argv[1]);

#else // Manual operation

    [[maybe_unused]] int startnum;

#endif

    asio::ip::tcp::socket socket{context};
    try
    {
        // Establish connection to server
        socket.connect(
            asio::ip::tcp::endpoint{asio::ip::make_address_v4(host), port});
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
    }

    while (true)
    {
#ifdef AUTO // Automatic operation
        for (auto i = start_num; i < start_num + nof_cmds_to_generate; ++i)
        {
            std::string s;
            s = std::string("Command") + std::to_string(i) + std::string("\n");
            send_string(socket, s);
            sleep(1);
        }
        send_string(socket, std::string(1, DISCONNECT));
        break;

#else // Manual operation
        std::string line;
        std::getline(std::cin, line);
        if (std::cin.eof())
        {
            send_string(socket, std::string(1, DISCONNECT));
            return 0;
        }
        line.append({'\n'});
        send_string(socket, line);
#endif
    }
}