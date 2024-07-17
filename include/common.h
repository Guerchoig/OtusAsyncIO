#pragma once
#include <string>
#include <thread>
#include <vector>

// #define DEBUG

std::string pid_to_string(std::thread *p);
std::string this_pid_to_string();

#ifdef DEBUG
std::string pid_to_string(std::thread *p);
#define _DS(s) std::cout << (this_pid_to_string() + " " + s + "\n");
#define _DF _DS(__PRETTY_FUNCTION__)
#else
#define _DS(s) ;
#define _DF ;
#endif

inline std::string pid_to_string(std::thread *p)
{
    return std::to_string(std::hash<std::thread::id>{}(p->get_id()));
}

inline std::string this_pid_to_string()
{
    return std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()));
}

// Cmds input buffer type(commands are strings) * /
using cmds_t = std::vector<std::string>;
using pcmds_t = cmds_t *;
