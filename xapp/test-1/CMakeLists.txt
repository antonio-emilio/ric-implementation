cmake_minimum_required(VERSION 3.10)

project(hello-xapp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# In a real project, you would find and link against the FlexRIC libraries.
# For this example, we'll assume the necessary headers are in an include path
# and the libraries are available for linking.
# find_package(FlexRIC REQUIRED)

add_executable(hello-xapp src/main.cpp)

# target_link_libraries(hello-xapp PRIVATE FlexRIC::e2)

install(TARGETS hello-xapp DESTINATION bin)
