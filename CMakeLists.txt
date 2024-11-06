cmake_minimum_required(VERSION 3.10)
project(monitoring_programm)

# Include Conan's toolchain if it exists
if(EXISTS "${CMAKE_BINARY_DIR}/conan_toolchain.cmake")
  include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
endif()

# Ensure Conan-managed dependencies are available
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}")
find_package(cjson REQUIRED)

# Add the executable
add_executable(monitoring_programm
    src/main.c
    src/metrics.c
    src/expose_metrics.c
)

# Link libraries for libprom, libpromhttp, and pthread
target_link_libraries(monitoring_programm
    cjson::cjson      # Link to the cJSON package installed by Conan
    /usr/local/lib/libprom.so
    /usr/local/lib/libpromhttp.so
    pthread
)

# Optionally, you can include the Conan-managed include directories, 
# but `find_package(cjson)` should already handle this.
# include_directories(${CMAKE_BINARY_DIR}/include)

