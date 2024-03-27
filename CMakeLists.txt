cmake_minimum_required(VERSION 3.0)

project(console_chess VERSION 1.0)

include_directories(src/)

add_executable(console_chess src/main.cpp src/Board.cpp src/Board.hpp src/Piece.cpp src/Piece.hpp src/Gamestate.cpp src/Gamestate.hpp src/Player.cpp src/Player.hpp src/Prompt.cpp src/Prompt.hpp src/Util.cpp src/Util.hpp src/Move.cpp src/Move.hpp src/ChessException.cpp src/ChessException.hpp src/StateFactory.cpp src/StateFactory.hpp src/MessageManager.hpp src/MessageManager.cpp src/Message.hpp src/Message.cpp src/Debug.hpp src/Warnings.hpp)

# Optimize compiled code. O0-worst, O3-best
set(CMAKE_CXX_FLAGS "-O3")

# # compiler flags: all possible warnings
# set(CMAKE_CXX_FLAGS "-pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror -Wno-unused")

# # Flags for creating an extra file for profiling
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
# SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
# SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")