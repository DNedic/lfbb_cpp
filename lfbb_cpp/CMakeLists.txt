# Library source CMakeLists
# Djordje Nedic 2022

cmake_minimum_required(VERSION 3.16)

add_library(${PROJECT_NAME} INTERFACE)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_11)

target_include_directories(${PROJECT_NAME}
INTERFACE
    inc
)
