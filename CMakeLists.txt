cmake_minimum_required(VERSION 3.5)
project(VKSender LANGUAGES CXX)

find_package(Qt5 COMPONENTS Widgets Core Gui Network NetworkAuth REQUIRED)

add_subdirectory(src)

