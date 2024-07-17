/**
 * @brief bulk_server.h Contains definitions for 'bulk_server' commands server,
 *        accessible by network
 */
#pragma once
#include "common.h"
#include "bulk_server.h"
#include "cmd_output.h"
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/detached.hpp>
#include <iostream>
#include <tuple>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <atomic>

namespace asio = boost::asio;

using tcp_t = asio::ip::tcp;
using port_t = asio::ip::port_type;

constexpr auto default_ip = "127.0.0.1";
constexpr port_t default_port = 4507;
constexpr size_t default_cmd_blk_size = 5;
constexpr char msg_end = '\n';

/**
 * @brief A type storing server params, given in the command string
 */
struct server_t
{
    std::string ip_addr;
    port_t port;
    size_t block_size;
};

/**
 * @brief Globally accessible server params item
 */
inline server_t server;

/**
 * @brief Globally accessible io_context item
 */
inline asio::io_context context;

/**
 * @brief Extracts port, block_size, ip_addr from command line
 * @param argc
 * @param argv
 * @param server_params
 * @return
 */
inline bool get_params(int argc, char **argv, server_t &server_params)
{

    bool res = true;

    if (argc == 2)
        if (strstr(argv[1], "help") != nullptr)
            argc = -1;

    switch (argc)
    {
    case 1:
        server_params.port = default_port;
        server_params.block_size = default_cmd_blk_size;
        server_params.ip_addr = std::string(default_ip);
        break;
    case 2:
        server_params.port = std::atoi(argv[1]);
        server_params.block_size = default_cmd_blk_size;
        server_params.ip_addr = std::string(default_ip);
        break;
    case 3:
        server_params.port = std::atoi(argv[1]);
        server_params.block_size = std::atoi(argv[2]);
        server_params.ip_addr = std::string(default_ip);
        break;
    case 4:
        server_params.port = std::atoi(argv[1]);
        server_params.block_size = std::atoi(argv[2]);
        server_params.ip_addr = argv[3];
        break;
    default:
        std::cout << "The use is: bulk_server <port number> <cmd block size> <ip address> \n"
                     "or\tbulk_server <port number> <cmd block size>\n"
                     "or\tbulk_server <port number>\n"
                     "or\tbulk_server\n"
                     "defaults: port number = 4507; block size = 5; ip address = 127.0.0.1 \n ";
        res = false;
        break;
    }
    return res;
}
