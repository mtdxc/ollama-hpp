cmake_minimum_required(VERSION 3.10.0)
project(ollama_talk VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 11)

include_directories(include)
add_executable(ollama_talk examples/talk.cc)

