cmake_minimum_required(VERSION 3.10)

project(cnake)

include(FetchContent)

add_compile_options(-Og -ggdb3 -fPIC -Wall)
set(CMAKE_CXX_STANDARD 17)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache) # Less useful to do it for linking, see edit2
endif(CCACHE_FOUND)


find_package(Boost REQUIRED COMPONENTS python)
find_package(Python COMPONENTS Development)

set(BUILD_TESTING OFF)
FetchContent_Declare(
    CMake
    URL https://github.com/Kitware/CMake/releases/download/v3.21.3/cmake-3.21.3.tar.gz
    URL_MD5 c0feb5855604f68b09bdb3acb623619e
)
FetchContent_MakeAvailable(CMake)

set_target_properties(cmake ctest cpack PROPERTIES EXCLUDE_FROM_ALL ON)

add_library(cnake SHARED "src/Bindings.cxx")
target_link_options(cnake PRIVATE "LINKER:--no-undefined")
target_link_libraries(cnake PRIVATE CMakeLib CPackLib Boost::python Python::Python)

target_include_directories(cnake PRIVATE
    inc
    ${cmake_SOURCE_DIR}/Source
    ${cmake_SOURCE_DIR}/Utilities/
    ${cmake_SOURCE_DIR}/Utilities/std/
    ${cmake_BINARY_DIR}/Utilities/
)

set_target_properties(cnake 
    PROPERTIES PREFIX ""
    OUTPUT_NAME "_cnake"
  #  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/py"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/py/cnake"
)