cmake_minimum_required(VERSION 3.14)
project(GoboardProject VERSION 1.0.0 LANGUAGES CXX)

# C++17規格を指定
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ライブラリのソースファイルを指定
set(GOBOARD_SOURCES
    src/Goboard.cpp
    src/StoneString.cpp
    src/ZobristTable.cpp
)

# goboard_lib という名前で静的ライブラリを作成
add_library(goboard_lib STATIC ${GOBOARD_SOURCES})

# インクルードディレクトリの設定 (パブリックとして公開)
target_include_directories(goboard_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# テストを有効化して、testディレクトリを読み込む
enable_testing()
add_subdirectory(test)