project(lazy_re C)

cmake_minimum_required(VERSION 2.6)

SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Os -Wall -Wextra")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Os")

add_library(lazy_re STATIC src/lazy_re.c)

option(UNIT_TESTING "Build unit tests" ON)

if (UNIT_TESTING)
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/cmocka/include")
    link_directories("${CMAKE_CURRENT_SOURCE_DIR}/cmocka/build/src")
    add_executable(re_test test/re.c)
    target_link_libraries(re_test lazy_re cmocka)
    set_target_properties(re_test
        PROPERTIES
        LINK_FLAGS "-Wl,--wrap=regcomp, -Wl,--wrap=regexec, -Wl,--wrap=regfree"
        )
endif (UNIT_TESTING)
