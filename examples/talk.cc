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

void testchat() {
    ollama::message message1("user", "What are nimbus clouds?");
    ollama::message message2("assistant", "Nimbus clouds are dense, moisture-filled clouds that produce rain.");
    ollama::message message3("user", "What are some other kinds of clouds?");
    ollama::chat("qwen2.5:7b", { message1, message2, message3 }, on_receive_response);
}

int main()
{
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    ollama::setServerURL("http://192.168.24.10:11434"); // Set the Ollama server URL
#endif
    std::string line;
    bool use_chat = true;
    std::shared_ptr<ollama::request> req;
    if (use_chat) {
        ollama::message message1("user", "What are nimbus clouds?");
        req = std::make_shared<ollama::request>("qwen2.5:7b", message1);
    }
    else {
        req = std::make_shared<ollama::request>("qwen2.5:7b", "作为一个中文导游，请以简短方式回答我");
    }
    signal(SIGINT, &signal_handler); // Register signal handler for Ctrl+C
    auto cb = [req](const ollama::response& response) -> bool {
        std::cout << response << std::flush;
        if (response.as_json().contains("context")) {
            (*req)["context"] = response.as_json()["context"];
        }
        if (response.as_json().contains("done") && response.as_json()["done"] == true) {
            std::cout << std::endl;
            return false;
        }
        return interrupted ? false : true; // Return true to continue streaming this response; return false to stop immediately.
    };

    while (1) {
        std::getline(std::cin, line);
        if(line == "exit" || line == "quit") break;

        interrupted = false;
        if (use_chat) {
            (*req)["messages"][0]["content"] = line;
            ollama::chat(*req, cb);
        }
        else {
            (*req)["prompt"] = line;
            ollama::generate(*req, cb);
        }
    }
    return 0;
}
