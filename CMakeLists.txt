cmake_minimum_required(VERSION 3.21)
project(Chip8_emulator_Qt)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_PREFIX_PATH "/usr/lib/qt5")


find_package(Qt5 COMPONENTS
        Core
        REQUIRED)
find_package(Qt5Widgets REQUIRED)

add_executable(Chip8_emulator_Qt src/main.cpp src/MainWindow.cpp src/RenderWindow.cpp src/EmuWindow.cpp src/Chip8.cpp)
target_link_libraries(Chip8_emulator_Qt
        Qt5::Core
        Qt5::Widgets
        )

