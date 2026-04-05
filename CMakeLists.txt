cmake_minimum_required(VERSION 3.28)
project(dynstr C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(dynstr STATIC src/dynstr.c)

if (MSVC)
    target_compile_options(dynstr PRIVATE /W4 /WX)
else ()
    target_compile_options(dynstr PRIVATE -Wall -Wextra -Werror)
endif ()

install(TARGETS dynstr ARCHIVE DESTINATION lib)
install(FILES src/dynstr.h DESTINATION include)

include(CTest)

if (BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
            cmocka
            URL https://cmocka.org/files/2.0/cmocka-2.0.2.tar.xz
            URL_HASH SHA256=39f92f366bdf3f1a02af4da75b4a5c52df6c9f7e736c7d65de13283f9f0ef416
            EXCLUDE_FROM_ALL
    )

    set(WITH_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(UNIT_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(cmocka)

    add_executable(test_dynstr
            tests/test_dynstr.c)
    target_link_libraries(test_dynstr PRIVATE dynstr cmocka)
    add_custom_command(TARGET test_dynstr POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:cmocka>
            $<TARGET_FILE_DIR:test_dynstr>
    )

    add_test(NAME dynstr_tests COMMAND test_dynstr)
endif ()
