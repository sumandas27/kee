cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 23)
project(kee)

add_subdirectory(externals/boost EXCLUDE_FROM_ALL)
add_subdirectory(externals/magic_enum)
add_subdirectory(externals/nativefiledialog-extended)
add_subdirectory(externals/raylib)
add_subdirectory(externals/raylib-cpp)

file(GLOB_RECURSE KEE_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(${PROJECT_NAME} ${KEE_SRC})

# /W4 /permissive-          - Enable all compile-time warnings.
# -Wall -Wextra -Wpedantic  - '' (Non-MSVC)
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/opt/homebrew/opt/llvm/lib/c++ -Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::optional Boost::json magic_enum nfd raylib raylib_cpp)