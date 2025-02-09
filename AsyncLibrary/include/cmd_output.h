/**
 * @brief cmd_output.h
 *        Contains output entities definitions for async library
 */
#pragma once
#include "async_internal.h"
#include <string>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <condition_variable>
#include <list>
#include <atomic>
#include <type_traits>
#include <memory>

constexpr unsigned int NOFLAGS = 0x00000000;
struct output_context_t;

/**
 * @brief Realizes flag "to file" for template functions
 */
struct TO_FILE
{
    const static unsigned int value = 0xFFFF0000;
};

/**
 * @brief Realizes flag "to console" for template functions
 */
struct TO_CONS
{
    const static unsigned int value = 0x0000FFFF;
};

/**
 * @brief A value to initialize timestamp of a command block
 */
constexpr unsigned int EMPTY_TIME = 0;

/**
 * @brief A block of commands is a collection of commands + some state info
 */
struct cmd_block_t
{

    clock_t timestamp = EMPTY_TIME;         // time stamp for naming a file
    std::atomic<unsigned int> output_state; // one of flags: TO_FILE, TO_CONS or NOFLAGS
    cmds_t cmds;                            // Commands collection

    cmd_block_t() : timestamp(EMPTY_TIME) { output_state.store(NOFLAGS); }

    cmd_block_t(cmd_block_t &other) : timestamp(other.timestamp)
    {
        output_state.store(other.output_state.load());

        for (auto c : other.cmds)
            cmds.emplace_back(c);
    }

    cmd_block_t(cmd_block_t &&other) : timestamp(other.timestamp)
    {
        output_state.store(other.output_state.load());

        for (auto c : other.cmds)
            cmds.emplace_back(c);
    }

    cmd_block_t(const cmds_t _cmds) : timestamp(clock())
    {
        output_state.store(NOFLAGS);
        for (auto c : _cmds)
            cmds.emplace_back(c);
    }
};

/**
 * @brief Thread worker function to output command blocks to console
 */
void thread_to_console();

/**
 * @brief Thread worker function to output command blocks to file
 */
void thread_to_file();

/**
 * @brief Output cmd blocks queue
 */
class cmd_blocks_q_t
{
private:
    std::list<cmd_block_t> lst;     // the output queue of blocks
    std::shared_mutex common_mtx;   // mutex used to obtain ownership of whole q for read or write
    std::mutex file_mtx;            // mutex used to allow only one 'file' thread to read queue at the same time
    std::condition_variable_any cv; // common_mtx-based condition variable for pop from q

public:
    void erase_push(cmds_t &block); // pushes a block into output queue and erases the blocks which had been written to both file and console

    template <typename T>
    bool fetch_blocks(std::vector<cmd_block_t> &blocks); // copy ready-to-be-written blocks for output them and establishes
                                                         // their output state in queue
    bool empty() { return lst.empty(); }                 // Incapsulation-only queue's emty func
};

/**
 * @brief Output threadpool
 */
struct out_threadpool_t
{
    std::vector<std::thread> pool;                 // the pool of output threads: one console thread and two file threads
    std::mutex mtx;                                // Output pool mutex, helps to lazy start output threads
    bool threads_started;                          // The flag helps to lazy start output threads with first 'connect' call
    const char *log_dir = nullptr;                 // A path to output files
    out_threadpool_t() : threads_started(false) {} // constructor
    void try_to_launch(const char *log_dir);       // The output threads lazy-start function
};

/**
 * @brief A buffer for static commands, common for all the connections
 */
struct static_cmds_buf_t
{
    std::mutex mtx;                                                    // buffer access mutex
    cmds_t cmds;                                                       // queue of cmds, static commands stay here before forming a block in the output queue
    size_t block_size;                                                 // common block size for all the connections
    static_cmds_buf_t(size_t _block_size) : block_size(_block_size) {} // constructor
    void save_static_cmd(const std::string buf);                       // put a command into static buffer;
                                                                       // if there are already block_size commands there  - then output to blocks queue
};

/**
 * @brief The type to aggregate all output objects
 */
struct output_context_t
{

    struct out_threadpool_t th_pool; // output threadpool
    static_cmds_buf_t static_cmds;   // buffer for input static cmds, common for all the connections
    cmd_blocks_q_t blocks_q;         // output cmd_blocks FIFO queue
    output_context_t(size_t _block_size) : static_cmds(_block_size) {}
};

/**
 * @brief An enveloppe for output_context_t to lazy-allocate it
 *
 */
struct output_ctxt
{
    output_context_t *ptr{nullptr};                     // pointer to lazy-allocate output_context_t
    output_context_t *operator()(size_t block_size = 0) // functor giving access to output_context_t*,
    {
        if (!ptr)
            ptr = new output_context_t(block_size);
        return ptr;
    }
    ~output_ctxt() { delete ptr; }
};

/**
 * @brief a global storage for output context
 */
inline output_ctxt output_context;
