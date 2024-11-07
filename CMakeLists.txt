cmake_minimum_required(VERSION 3.10)
project(monitoring_programm)

# Run Conan install if needed (ensure files are placed in the build directory)
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan_toolchain.cmake")
    message(STATUS "Running Conan install...")
    execute_process(COMMAND conan install .. --output-folder ${CMAKE_BINARY_DIR} --build=missing
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    RESULT_VARIABLE conan_result)
    if(NOT conan_result EQUAL 0)
        message(FATAL_ERROR "Conan install failed")
    endif()
endif()

# Load Conan’s toolchain
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type" FORCE)
endif()

# Add the executable
add_executable(monitoring_programm
    src/main.c
    src/metrics.c
    src/expose_metrics.c
)

# Find the cJSON package from Conan (using the CMakeDeps generator)
find_package(cJSON REQUIRED)

# Include the cJSON header directories (this should be automatically handled by find_package)
target_include_directories(monitoring_programm PRIVATE ${CJSON_INCLUDE_DIRS})

# Link libraries
target_link_libraries(monitoring_programm
    cjson::cjson
    /usr/local/lib/libprom.so
    /usr/local/lib/libpromhttp.so
    pthread
)
