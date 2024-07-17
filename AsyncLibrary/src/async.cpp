/**
 * @brief async.cpp - contains the input part of 'async' commands processing library
 *                    which serve to group commands from multiple connections into blocks
 *                    and put them into a single output queue -block-by-block
 */

#include "async_internal.h"
#include "cmd_output.h"
#include "async.h"
#include "common.h"
#include <chrono>
#include <string>
#include <thread>
#include <utility>
#include <mutex>
#include <queue>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <unistd.h>

/**
 * @brief Thread-safe delete of connection
 * @param ch connection handle
 * @return true if the connection was in pool and was deleted
 */
bool input_connections_t::delete_connection(connection_handle_t ch)
{
    std::lock_guard g(mtx);
    if (ctxs.contains(ch))
    {
        ctxs.erase(ch);

        return true;
    }
    return false;
}

/**
 * @brief tread-safe empty() function for input connections pool
 * @return
 */
bool input_connections_t::empty()
{
    std::lock_guard(input_connections.mtx);
    return input_connections.ctxs.empty();
}

/**
 * @brief Process input command to create lexema (token + command)
 * @param buf The buffer, which command comes from
 * @return Created lexema
 */
lexema_t make_lexema(const std::string buf)
{
    if (buf.find(open_br_sym) != buf.npos)
        return std::make_pair(OpenBr, std::string(1, open_br_sym));
    if (buf.find(close_br_sym) != buf.npos)
        return std::make_pair(CloseBr, std::string(1, close_br_sym));
    return std::make_pair(Cmd, buf);
}

/**
 * @brief Thread-safely save a 'static' cmd to the 'cmds' buffer
 *        output the whole 'cmds' to output blocks queue, when it grows appropriate size
 * @param buf buffer, containing the new 'static' cmd
 */
void static_cmds_buf_t::save_static_cmd(const std::string buf)
{
    std::lock_guard g(mtx);
    cmds.emplace_back(buf);
    if (cmds.size() == block_size)
        output_context()->blocks_q.erase_push(cmds); // Put into output q
}

/**
 * @brief Namespace for library interface
 */
namespace edit
{

    /**
     * @brief Creates new connection to input commands queue
     * @param block_size - nof cmds in command block
     * @return a handle to the created connection
     */
    connection_handle_t connect(std::size_t block_size, const char *log_dir)
    {
        connection_handle_t handle;
        {
            std::lock_guard lock(input_connections.mtx);

            // Add new connection handle to the set of connections
            handle = input_connections.ctxs.emplace(std::make_pair(input_connections.ctxs.size(), new input_context_t(block_size))).first->first;
        }

        // Launch output threads if they are not launched yet
        output_context(block_size)->th_pool.try_to_launch(log_dir);

        return handle;
    }

    /**
     * @brief Receives exactly one command and put it into static or dynamic queue
     * @param ch Handle for connection, created by connect
     * @param buf Buffer containing the command
     */
    void receive(connection_handle_t ch, const std::string buf)
    {

        if (!buf.size())
            return;
        auto inp_ctx = input_connections.ctxs[ch];

        auto lexema = make_lexema(buf);
        int lex_id = lexema.first; // lexema.first: Lex enum,
                                   // lexema.second: command string, if any, or ""
        switch (lex_id)
        {
        case Cmd: // command received
            if (inp_ctx->dynamic_depth == 0)
                output_context()->static_cmds.save_static_cmd(buf); // put it into common static q
            else
                inp_ctx->dyna_cmds.emplace_back(lexema.second); // put it into local dynamic q
            break;
        case OpenBr:                    // '{'
            (inp_ctx->dynamic_depth)++; // nested '{' are accounted to errorlessly accept nested '}'
            break;
        case CloseBr: // '}'
            if ((--(inp_ctx->dynamic_depth)) < 0)
            { // unpair close bracket!
                std::cerr << "Unpair close bracket" << std::endl;
                std::quick_exit(2);
            }
            if ((inp_ctx->dynamic_depth) == 0)                             // dynamic block is finishing
                output_context()->blocks_q.erase_push(inp_ctx->dyna_cmds); // Put block into output q and clear it
            break;
        default:
            std::cerr << "Unknown command" << std::endl;
            std::quick_exit(2);
            break;
        };
    }

    /**
     * @brief Delete connection corresponding to the given handle;
     *        forms a block from the rest of input cmd queue
     *        and pushes it into output blocks queue
     * @param ch Handle for connection, created by 'connect'
     */
    void disconnect(connection_handle_t ch)
    {

        auto inp_ctx = input_connections.ctxs[ch];

        // Push the last block to output queue
        output_context()->blocks_q.erase_push(inp_ctx->dyna_cmds);

        // Delete connection
        input_connections.delete_connection(ch);
    }

    /**
     * @brief Calls disconnect for every connection
     *        pushes the rest of static cmds buffer(queue) into output blocks queue
     */
    void terminate()
    {
        for (auto cn : input_connections.ctxs)
            disconnect(cn.first);
        output_context()->blocks_q.erase_push(output_context()->static_cmds.cmds);
        sleep(1);
    }
}