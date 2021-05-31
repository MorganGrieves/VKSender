cmake_minimum_required(VERSION 3.5)
project(VKSender LANGUAGES CXX)

find_package(Qt5 COMPONENTS Widgets Core Gui Network NetworkAuth WebKit WebKitWidgets REQUIRED)

add_subdirectory(src)

add_subdirectory(install)

# add_compile_options(-Wall -Wextra -Wpedantic -Wnon-virtual-dtor)
# add_compile_options(-static -static-libgcc -static-libstdc++)
# set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install/data CACHE PATH "CMAKE_INSTALL_PREFIX" FORCE)

