cmake_minimum_required(VERSION 3.10.0)

project(minim)
#output to bin folder with all scripts
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wno-narrowing )

add_definitions( -D_SRC_MINIM_EMU=1 )

add_subdirectory(deps/libpd)
add_subdirectory(deps/rtmidi)
add_subdirectory(deps/rtaudio)
add_subdirectory(deps/raylib)

add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/audio.cpp 
    ../minim_hw/api/gfx.cpp 
    ../minim_hw/api/cmd.cpp
    ../minim_hw/api/gfx/fonttiny.cpp
)

target_include_directories(${PROJECT_NAME} 
	PUBLIC include
    PUBLIC ../minim_hw/api
    PUBLIC ../minim_hw/api/gfx
    PUBLIC deps/libpd/pure-data/src 
    PUBLIC deps/libpd/libpd_wrapper
    PUBLIC deps/rtmidi
    PUBLIC deps/rtaudio
    PUBLIC deps/raylib/src
)

target_link_directories(${PROJECT_NAME} 
    PUBLIC deps/libpd
    PUBLIC deps/rtmidi
    PUBLIC deps/rtaudio
    PUBLIC deps/raylib
)

target_link_libraries(${PROJECT_NAME} 
    libpd 
    rtmidi
    rtaudio
    raylib    
)


# Checks if OSX and links appropriate frameworks (only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()