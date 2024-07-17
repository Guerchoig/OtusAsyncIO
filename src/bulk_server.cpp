/**
 * @brief bulk_server.cpp Contains realization for 'bulk_server' commands server,
 *        accessible by network
 */
#include "common.h"
#include "async.h"
#include "bulk_server.h"
#include "cmd_output.h"
#include <cstdlib>
#include <memory>
#include <utility>
#include <string>
#include <filesystem>

/**
 * @brief Handles CTRL-C signal to softly shutdown the server
 * @param _signal
 */
void SIGINT_handler([[maybe_unused]] int _signal)
{
    context.stop(); // stop the coro loop
}

/**
 * @brief A coro to process the input from connected client
 * @param _socket the socket corresponding to the client
 * @param handle connection handle
 * @param block_size commands block size
 * @return nothing
 */
asio::awaitable<void> run_session(tcp_t::socket _socket, edit::connection_handle_t handle, [[maybe_unused]] size_t block_size)
{

    constexpr size_t buf_size = 1024;
    std::string cmd;
    std::string line(buf_size, '\0');

    while (true)
    {

        [[maybe_unused]] auto n_read = co_await _socket.async_read_some(asio::buffer(line), asio::use_awaitable);
        if (!n_read)
        {
            std::cerr << "Zero bytes read " << "\n";
            quick_exit(1);
        }

        // Begin processing \n - delimited input string
        std::string_view s(line.begin(), line.begin() + n_read);

        // Process DISCONNECT symbol, received from client
        auto pos = line.find(edit::DISCONNECT);
        bool disconnect = (pos != std::string::npos);
        if (disconnect)
            s = s.substr(0, pos);

        // There can be several \n - delimited commands in the input string
        // or/and an unfinished command whith no delimiter at the end
        auto prev_pos = pos = 0;

        while (prev_pos < s.size())
        {
            pos = s.find("\n", prev_pos);
            if (pos == std::string::npos)
            {
                cmd.append(s, prev_pos, s.size() - prev_pos);
                break;
            }
            cmd.append(s, prev_pos, pos - prev_pos);
            prev_pos = pos + 1;
            edit::receive(handle, cmd);
            cmd.clear();
        }
        // On DISCONNECT close socket and return
        if (disconnect)
        {
            try
            {
                _socket.close();
            }
            catch (const std::exception &ex)
            {
                std::cerr << "Exception: " << ex.what() << "\n";
                quick_exit(1);
            }
            edit::disconnect(handle);
            std::cout << "disconnected " << handle << "\n";
            co_return;
        }
    }
}

/**
 * @brief A coro to process connection request from clients
 *        establishes connection and run session coro for it
 * @param context asio io_context
 * @param server server parameters structure
 * @return nothing
 */
asio::awaitable<void> run_server(asio::io_context &context, server_t &server)
{

    try
    {
        tcp_t::acceptor acceptor(context, tcp_t::endpoint{asio::ip::make_address_v4(server.ip_addr), server.port});
        while (true)
        {
            tcp_t::socket client = co_await acceptor.async_accept(asio::use_awaitable);
            auto handle = edit::connect(server.block_size);

            std::cout << "connected " << handle << "\n";
            asio::co_spawn(context, run_session(std::move(client), handle, server.block_size), asio::detached);
        }
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
    }
    auto a = 0;
    (void)a;
}

/**
 * @brief Menages log-directory before server start
 */
void clean_directory()
{
    // Clear log directory and console
    using namespace std::filesystem;
    auto res = system("clear");
    (void)res;

    if (!exists(edit::log_directory))
    {
        auto res = create_directory(edit::log_directory);
        assert(res);
    }

    const std::filesystem::directory_iterator _end;
    for (std::filesystem::directory_iterator it(edit::log_directory); it != _end; ++it)
        std::filesystem::remove(it->path());
}

/**
 * @brief Prepare log dir, establish SIGINT signal handler, starts server coro
 *        CTRL-C terminates operation
 * @param argc - nof parameters
 * @param argv - port, block_size, ip_addr  (consecutively-optional)
 * @return
 */
int main(int argc, char **argv)
{

    clean_directory();
    if (!get_params(argc, argv, server))
        return 0;

    std::cout << "running at " + server.ip_addr << ":" << server.port << "; block size = " << server.block_size << "\n";

    // Start server coro
    asio::co_spawn(context, run_server(context, server), asio::detached);

    // Establish CTRL-C handler
    struct sigaction handler;
    handler.sa_handler = SIGINT_handler;
    sigemptyset(&handler.sa_mask);
    handler.sa_flags = 0;
    sigaction(SIGINT, &handler, NULL);

    // Starts coro loop
    context.run();

    // Accurately terminates server
    edit::terminate();
}
