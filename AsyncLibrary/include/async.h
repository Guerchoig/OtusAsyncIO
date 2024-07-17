/**
 * @brief async.h - async library interface
 */

#pragma once
#include <memory>
#include <queue>
#include <condition_variable>

namespace edit
{

    using connection_handle_t = size_t;
    /**
     * @brief A default value for output directory
     */
    constexpr auto log_directory = "./log";

    /**
     * @brief A symbol, which must be sent to disconnect from server
     */
    constexpr unsigned char DISCONNECT = 0x4;

    /**
     * @brief Creates new connection to input commands queue
     * @param block_size - nof cmds in command block
     * @return a handle to the created connection
     */
    connection_handle_t connect(std::size_t block_size, const char *log_dir = log_directory);

    /**
     * @brief Receives exactly one command and put it into cmd input queue
     * @param ch Handle for connection, created by connect
     * @param buf Buffer containing the command
     */
    void receive(connection_handle_t ch, const std::string buf);

    /**
     * @brief Delete connection corresponding to the given handle;
     *        form a block from the rest of input cmd queue
     *        and pushes it into output blocks queue
     *        If that was the last connection, then
     *        - initiate termination of output threads
     * @param ch Handle for connection, created by connect
     */
    void disconnect(connection_handle_t ch);

    /**
     * @brief Call 'disconnect' for every connection
     *        and put the rest of static commands buffer into output blocks queue
     */
    void terminate();
}