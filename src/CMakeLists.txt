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
    RUNTIME DESTINATION ${RUNTIME_DESTINATION}
)

install(CODE
    "execute_process(
        COMMAND
            ${WINDEPLOYQT_APP} ${CMAKE_INSTALL_PREFIX}/${RUNTIME_DESTINATION}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}
    )")


install_qt4_executable(
    "${RUNTIME_DESTINATION}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}"
)
