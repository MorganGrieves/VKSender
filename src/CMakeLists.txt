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

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${UI} ${RESOURCE})

target_link_libraries(
    ${PROJECT_NAME}
    Qt5::Network
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    Qt5::NetworkAuth
    Qt5::WebKitWidgets
    Qt5::WebKit
    )

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${RUNTIME_DESTINATION}
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
