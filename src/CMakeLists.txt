cmake_minimum_required(VERSION 3.4)
set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE TYPE INTERNAL FORCE)

project(Sudoku)

set(LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libraries CACHE PATH "Path to lib folder")

set(GLFW_INCLUDE_DIRS ${LIBRARY_DIR}/glfw/include CACHE PATH "Path to glfw include")
set(GLFW_LIBRARY ${LIBRARY_DIR}/glfw/lib-vc2019/glfw3.lib CACHE PATH "Path to glfw include")

set(GLEW_INCLUDE_DIRS ${LIBRARY_DIR}/glew/include CACHE PATH "Path to glew include")
set(GLEW_LIBRARY ${LIBRARY_DIR}/glew/lib/Release/x64/glew32s.lib CACHE PATH "Path to glew libraries")

include(opencv.cmake)

find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)

include_directories( ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
file(GLOB Sudoku_SOURCES ./*.cpp
			     ./*.h)

add_executable(Sudoku ${Sudoku_SOURCES})
target_link_libraries (Sudoku ${GLEW_LIBRARY} ${GLFW_LIBRARY} ${OPENGL_LIBRARIES} ${OpenCV_LIBS}) 