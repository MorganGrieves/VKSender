
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file (GLOB_RECURSE SOURCES "*.cpp")
file (GLOB_RECURSE HEADERS "*.h")
file (GLOB_RECURSE UI "*.ui")
file (GLOB_RECURSE RESOURCE "*.qrc")

add_executable(VKSender ${HEADERS} ${SOURCES} ${UI} ${RESOURCE})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

target_link_libraries(
    ${PROJECT_NAME}
    Qt5::Network
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    Qt5::NetworkAuth
    )
