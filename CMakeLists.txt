cmake_minimum_required(VERSION 3.16)
project(MatronixLauncher LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Qt6 (Windows/macOS/Linux) — используется для интерфейса и сети
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/AuthDialog.cpp
    src/VersionManager.cpp
    src/ModManager.cpp
    src/LaunchManager.cpp
    src/AccountManager.cpp
)

set(HEADERS
    src/MainWindow.h
    src/AuthDialog.h
    src/VersionManager.h
    src/ModManager.h
    src/LaunchManager.h
    src/AccountManager.h
)

set(FORMS
    src/ui/MainWindow.ui
    src/ui/AuthDialog.ui
)

set(RESOURCES
    src/resources.qrc
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Widgets Qt6::Network
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    QT_NO_KEYWORDS
    MATRONIX_VERSION="${PROJECT_VERSION}"
    MATRONIX_AUTH_SERVER="https://authserver.ely.by"
    MATRONIX_ACCOUNT_SERVER="https://account.ely.by"
)

# Иконка для Windows (встроенная через资源 / rc)
if(MSVC)
    target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.ico)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
