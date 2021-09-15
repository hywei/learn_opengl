cmake_minimum_required (VERSION 3.2)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(learn_opengl)

include_directories(
  # 3rd include files
  ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/include
)

link_directories(
  # 3rd lib files
  ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/lib

)

# The following lines build the main executable. If you add a source
# code file to learn_opengl, be sure to include it in this list.
add_executable(learn_opengl

  # Header files
  src/shader.h
  src/camera.h
  src/light.h
  src/mesh.h

  # Source code files
  src/main.cpp
  src/shader.cpp
  src/mesh.cpp
  src/glad.c
)

target_link_libraries(learn_opengl glfw3 assimp-vc142-mt)

set_target_properties( learn_opengl
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
