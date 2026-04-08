cmake_minimum_required(VERSION 3.25)

# set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(IMGUI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/externals/imgui)

project(i8080 VERSION 0.0.1)

# include(${CMAKE_SOURCE_DIR}/cmake/FindSDL2.cmake)


include_directories(
    "${PROJECT_SOURCE_DIR}/externals/imgui"
)

include(FetchContent)

FetchContent_Declare(
	SDL2
	GIT_REPOSITORY https://github.com/libsdl-org/SDL
	GIT_TAG release-2.24.0
	GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(SDL2)

# find_package(SDL2)
find_package(OpenGL)

add_library(imgui
  ${IMGUI_INCLUDE_DIR}/imgui.cpp 
  ${IMGUI_INCLUDE_DIR}/imgui_demo.cpp
  ${IMGUI_INCLUDE_DIR}/imgui_draw.cpp 
  ${IMGUI_INCLUDE_DIR}/imgui_tables.cpp 
  ${IMGUI_INCLUDE_DIR}/imgui_widgets.cpp
  ${IMGUI_INCLUDE_DIR}/misc/cpp/imgui_stdlib.cpp
  ${IMGUI_INCLUDE_DIR}/backends/imgui_impl_sdl2.cpp
  ${IMGUI_INCLUDE_DIR}/backends/imgui_impl_sdlrenderer2.cpp)

target_include_directories(imgui PUBLIC ${sdl2_SOURCE_DIRS}/include ${IMGUI_INCLUDE_DIR})

add_executable(${PROJECT_NAME} 
	${PROJECT_SOURCE_DIR}/src/main.cpp ${PROJECT_SOURCE_DIR}/src/disassembler.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC 
	${IMGUI_INCLUDE_DIR}/
	${IMGUI_INCLUDE_DIR}/backends)

target_link_libraries(${PROJECT_NAME} PUBLIC
	SDL2main SDL2-static	
	imgui
	${OPENGL_gl_LIBRARY})
