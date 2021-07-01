set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file (GLOB_RECURSE SOURCES "*.cpp")
file (GLOB_RECURSE HEADERS "*.h")
file (GLOB_RECURSE UI "*.ui")
file (GLOB_RECURSE RESOURCE "*.qrc" "*.rc")

if (WIN32)
    set(WIN WIN32)
endif()

add_executable(${PROJECT_NAME} ${WIN} ${HEADERS} ${SOURCES} ${UI} ${RESOURCE})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

#include_directories(${CRYPTO_INCLUDE_DIRS})
target_link_libraries(
    ${PROJECT_NAME}
    Qt5::Network
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    Qt5::NetworkAuth
    Qt5::WebKitWidgets
    Qt5::WebKit
    OpenSSL::Crypto
    OpenSSL::SSL
    )

set(RUNTIME_DESTINATION "packages/vksender/data")

find_program(WINDEPLOYQT_APP WINDEPLOYQT)
if (NOT WINDEPLOYQT_APP)
    message(FATAL_ERROR "WINDEPLOYQT is not found!")
endif()

install(
    TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${RUNTIME_DESTINATION}/lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/${RUNTIME_DESTINATION}/lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${RUNTIME_DESTINATION}
)

get_target_property(QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
exec_program(
    ${QMAKE_EXECUTABLE} ARGS -query QT_INSTALL_PLUGINS
    OUTPUT_VARIABLE QT_PLUGIN_PATH
)

set(dirs)
foreach(path ${CMAKE_PREFIX_PATH})
    if (WIN32)
       list(APPEND dirs ${path} ${path}/bin)
    else()
        list(APPEND dirs ${path} ${path}/lib)
    endif()
endforeach()

set(plugs)
set(openssl_LIBS)
if(WIN32)
    foreach(plug qwindows)
        string(TOUPPER ${plug} UPPERCASE_PLUG)
        set(QT_${UPPERCASE_PLUG}_PLUGIN_RELEASE ${QT_PLUGIN_PATH}/platforms/${plug}.dll)
        list(APPEND plugs ${plug})
    endforeach()

    foreach(plug qwindowsvistastyle)
        string(TOUPPER ${plug} UPPERCASE_PLUG)
        set(QT_${UPPERCASE_PLUG}_PLUGIN_RELEASE ${QT_PLUGIN_PATH}/styles/${plug}.dll)
        list(APPEND plugs ${plug})
    endforeach()

    list(APPEND openssl_LIBS ${CMAKE_PREFIX_PATH}/bin/libcrypto-1_1-x64.dll)

    list(APPEND openssl_LIBS ${CMAKE_PREFIX_PATH}/bin/libssl-1_1-x64.dll)
endif()

install(
    FILES ${openssl_LIBS}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${RUNTIME_DESTINATION})

install_qt4_executable(
    "${RUNTIME_DESTINATION}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}"
    "${plugs}"
    ""
    "${dirs}"

)
