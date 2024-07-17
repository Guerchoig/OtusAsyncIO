/**
 * @brief async_internal.h - async library internal definitions for commands input
 */

#pragma once
#include "async.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <thread>

using namespace edit;

// Special symbols for input cmd stream
constexpr unsigned char open_br_sym = '{';
constexpr unsigned char close_br_sym = '}';

/**
 * @brief Tokens for lexemas
 */
enum Lex : unsigned int
{
    OpenBr,  // open bracket received
    CloseBr, // close bracket received
    Cmd      // command
};

/**
 * @brief Lexema type
 */
using lexema_t = std::pair<enum Lex, std::string>;

/**
 * @brief Cmds input buffer type (commands are strings)
 */
using cmds_t = std::vector<std::string>;

/**
 * @brief An input context; provide input block size and realize input queue
 */
struct input_context_t
{
    size_t block_size; // command block size
    cmds_t dyna_cmds;  // dynamic commands stay here before they form a block
    int dynamic_depth; // needed to follow using of brackets
    explicit input_context_t(size_t block_size) : block_size(block_size), dynamic_depth{0} {}
};

/**
 * @brief A ptr to input context type
 */
using sp_input_context_t = std::shared_ptr<input_context_t>;

/**
 * @brief A type for pool of input connections
 */
struct input_connections_t
{
    std::mutex mtx;                                                   // A mutex for calling input interface methods from multiple threads
    std::unordered_map<connection_handle_t, sp_input_context_t> ctxs; // connections pool
    ~input_connections_t()                                            // Thread-safe destructor
    {
        std::lock_guard g(mtx);
        ctxs.clear();
    }
    bool delete_connection(connection_handle_t ch); // deletes a conntection from 'ctxs' by handle
    bool empty();                                   // Thread-safe 'empty' indicator
};

/**
 * @brief A pool of input connections
 */
inline input_connections_t input_connections;

lexema_t make_lexema(const std::string buf);
