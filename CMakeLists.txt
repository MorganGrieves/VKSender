cmake_minimum_required(VERSION 3.5)
project(VKSender LANGUAGES CXX)

include(CMakePrintHelpers)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OPENSSL_USE_STATIC_LIBS TRUE)

find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui Network NetworkAuth WebKit WebKitWidgets REQUIRED)
find_package(OpenSSL REQUIRED)

include(GNUInstallDirs)
include(DeployQt4)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/installer/qt-ifw CACHE PATH "CMAKE_INSTALL_PREFIX" FORCE)

add_subdirectory(src)

add_compile_options(-Wall -Wextra -Wpedantic -Wnon-virtual-dtor)

