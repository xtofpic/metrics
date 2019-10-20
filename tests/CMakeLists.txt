
include_directories (
	${TEST_SOURCE_DIR}
)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/externals/cmake-codecov/cmake" ${CMAKE_MODULE_PATH})

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable (010-TestCase 010-TestCase.cpp)

set_property(TARGET 010-TestCase PROPERTY CXX_STANDARD 11)

add_test(test1 ${CMAKE_CURRENT_BINARY_DIR}/bin/010-TestCase)

if(METRIC_ENABLE_COVERAGE)
    find_package(codecov)
    add_coverage(010-TestCase)
endif()





