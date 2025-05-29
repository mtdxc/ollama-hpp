#include "ollama.hpp"
#include "json.hpp"
#include <iostream>
#include <functional>

#include <csignal>
#include <atomic>

std::atomic<bool> interrupted(false);

void signal_handler(int signal) {
    if (signal == SIGINT) {
        interrupted = true;
        std::cout << "\n[Interrupted by user]\n";
    }
}

bool on_receive_response(const ollama::response& response)
{   
    std::cout << response << std::flush;
    bool done = false;
    if (response.as_json()["done"]==true) { done=true;  std::cout << std::endl;}
    if (interrupted) return false;
    return !done; // Return true to continue streaming this response; return false to stop immediately.
}

int main()
{
    std::string line;
    ollama::request req("qwen3", "请以简短的方式回答我!");
    signal(SIGINT, &signal_handler); // Register signal handler for Ctrl+C
    while (1) {
        std::getline(std::cin, line);
        if(line == "exit" || line == "quit") break;
        req["prompt"] = line;
        interrupted = false;
        ollama::generate(req, [&req](const ollama::response& response) {
            std::cout << response << std::flush;
            if (response.as_json().contains("context")) {
                req["context"] = response.as_json()["context"];
            }
            if (response.as_json()["done"]==true) { 
                std::cout << std::endl;
                return false;
            }
            return interrupted?false:true; // Return true to continue streaming this response; return false to stop immediately.
        });
    }
    return 0;
}
