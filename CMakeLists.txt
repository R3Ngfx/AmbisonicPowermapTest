cmake_minimum_required(VERSION 3.10)
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

project(ambisonicTest)

add_subdirectory(Spatial_Audio_Framework)

include_directories(${PROJECT_NAME} PRIVATE powermap)

set(SOURCES ambisonicTest.c powermap/_common.h powermap/powermap.h powermap/powermap_internal.h powermap/powermap_internal.c powermap/powermap.c tinywav/tinywav.h tinywav/tinywav.c stb_image_write.h)

add_executable(ambisonicTest ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE saf -lm)
