cmake_minimum_required(VERSION 2.8)

# guard against in-source builds (got this from Eigen)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt. ")
endif()

#add the customised package searches to the module path
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(CheckCXXCompilerFlag)

# c++11 is required to compile this properly
check_cxx_compiler_flag(-std=c++11 HAS_CPP11)
if(HAS_CPP11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else(HAS_CPP11)
    message(FATAL_ERROR "Requires C++11 compatible compiler")
endif(HAS_CPP11)

# now add other compiler flags
macro(add_cxx_compiler_flag FLAG)
    string(REGEX REPLACE "-" "" SFLAG1 ${FLAG})
    string(REGEX REPLACE "\\+" "p" SFLAG ${SFLAG1})
    check_cxx_compiler_flag(${FLAG} COMPILER_SUPPORT_${SFLAG})
    if(COMPILER_SUPPORT_${SFLAG})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
    endif()
endmacro(add_cxx_compiler_flag)

add_cxx_compiler_flag("-pedantic")
add_cxx_compiler_flag("-Wall")
add_cxx_compiler_flag("-Wextra")
add_cxx_compiler_flag("-Wfatal-errors")

# libraries required
find_package(CFITSIO REQUIRED)
find_package(HEALPix REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options log thread)
find_package(Threads)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${HEALPix_INCLUDE_DIR})

add_executable(Cat2Map Cat2Map.cpp)
target_link_libraries(Cat2Map ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${HEALPix_LIBRARIES} ${CFITSIO_LIBRARIES})
