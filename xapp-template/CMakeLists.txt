cmake_minimum_required(VERSION 3.16)
project(smart_monitor_xapp C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")

# FlexRIC path
if(NOT FLEXRIC_PATH)
    set(FLEXRIC_PATH "/usr/local")
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${FLEXRIC_PATH}/include
    ${FLEXRIC_PATH}/include/flexric
    /usr/local/include
)

# Find required packages
find_package(PkgConfig REQUIRED)

# Find SQLite3
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

# Find JSON-C
pkg_check_modules(JSON_C REQUIRED json-c)

# Find libcurl
pkg_check_modules(CURL REQUIRED libcurl)

# Find pthread
find_package(Threads REQUIRED)

# Find math library
find_library(MATH_LIBRARY m)

# FlexRIC libraries
set(FLEXRIC_LIBRARIES
    ${FLEXRIC_PATH}/lib/libflexric.so
    ${FLEXRIC_PATH}/lib/libsm_kpm.so
    ${FLEXRIC_PATH}/lib/libsm_rc.so
    ${FLEXRIC_PATH}/lib/libsm_mac.so
    ${FLEXRIC_PATH}/lib/libsm_rlc.so
    ${FLEXRIC_PATH}/lib/libsm_pdcp.so
    ${FLEXRIC_PATH}/lib/libsm_gtp.so
)

# Check if FlexRIC libraries exist
foreach(LIB ${FLEXRIC_LIBRARIES})
    if(NOT EXISTS ${LIB})
        message(WARNING "FlexRIC library not found: ${LIB}")
        message(WARNING "Consider using simplified build without FlexRIC integration")
    endif()
endforeach()

# Source files
set(SOURCES
    src/smart_monitor_xapp.c
    src/analytics.c
    src/database.c
    src/utils.c
)

# Create main executable
add_executable(smart_monitor_xapp ${SOURCES})

# Link libraries
target_link_libraries(smart_monitor_xapp
    ${SQLITE3_LIBRARIES}
    ${JSON_C_LIBRARIES}
    ${CURL_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${MATH_LIBRARY}
)

# Try to link FlexRIC libraries if available
foreach(LIB ${FLEXRIC_LIBRARIES})
    if(EXISTS ${LIB})
        target_link_libraries(smart_monitor_xapp ${LIB})
    endif()
endforeach()

# Compiler and linker flags
target_compile_options(smart_monitor_xapp PRIVATE
    ${SQLITE3_CFLAGS_OTHER}
    ${JSON_C_CFLAGS_OTHER}
    ${CURL_CFLAGS_OTHER}
)

target_link_options(smart_monitor_xapp PRIVATE
    ${SQLITE3_LDFLAGS_OTHER}
    ${JSON_C_LDFLAGS_OTHER}
    ${CURL_LDFLAGS_OTHER}
)

# Install target
install(TARGETS smart_monitor_xapp
    RUNTIME DESTINATION bin
)

# Install configuration files
install(DIRECTORY config/
    DESTINATION share/smart_monitor_xapp/config
)

# Install documentation
install(FILES README.md
    DESTINATION share/smart_monitor_xapp/docs
)

# Test option
option(BUILD_TESTS "Build test executables" ON)

if(BUILD_TESTS)
    # Test source files
    set(TEST_SOURCES
        tests/test_analytics.c
        tests/test_database.c
        src/analytics.c
        src/database.c
        src/utils.c
    )
    
    # Test executables
    add_executable(test_analytics 
        tests/test_analytics.c
        src/analytics.c
        src/utils.c
    )
    
    add_executable(test_database
        tests/test_database.c
        src/database.c
        src/utils.c
    )
    
    # Link test libraries
    target_link_libraries(test_analytics
        ${SQLITE3_LIBRARIES}
        ${JSON_C_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${MATH_LIBRARY}
    )
    
    target_link_libraries(test_database
        ${SQLITE3_LIBRARIES}
        ${JSON_C_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${MATH_LIBRARY}
    )
    
    # Custom target for all tests
    add_custom_target(tests
        DEPENDS test_analytics test_database
    )
endif()

# Create simplified version without FlexRIC if libraries are missing
add_executable(smart_monitor_xapp_simple ${SOURCES})

target_link_libraries(smart_monitor_xapp_simple
    ${SQLITE3_LIBRARIES}
    ${JSON_C_LIBRARIES}
    ${CURL_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${MATH_LIBRARY}
)

target_compile_definitions(smart_monitor_xapp_simple PRIVATE
    -DSIMPLIFIED_BUILD
)

# Print configuration summary
message(STATUS "=== Smart Monitor xApp Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "FlexRIC path: ${FLEXRIC_PATH}")
message(STATUS "SQLite3 version: ${SQLITE3_VERSION}")
message(STATUS "JSON-C version: ${JSON_C_VERSION}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "========================================")