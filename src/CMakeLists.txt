cmake_minimum_required(VERSION 3.16)

project(cm5-programming-jig-gpio-config VERSION 1.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBGPIODCXX REQUIRED IMPORTED_TARGET libgpiodcxx)

add_library(cm5_led_gpio STATIC
    led_gpio.cpp
)
target_include_directories(cm5_led_gpio PUBLIC include)
target_link_libraries(cm5_led_gpio PUBLIC PkgConfig::LIBGPIODCXX)

add_library(cm5_led_protocol STATIC
    protocol.cpp
)
target_include_directories(cm5_led_protocol PUBLIC include)

add_executable(cm5-provisioner-led daemon_main.cpp)
target_link_libraries(cm5-provisioner-led PRIVATE cm5_led_gpio cm5_led_protocol)

add_executable(cm5_led_blink client_main.cpp)
target_link_libraries(cm5_led_blink PRIVATE cm5_led_protocol)

install(TARGETS cm5-provisioner-led cm5_led_blink RUNTIME DESTINATION bin)
