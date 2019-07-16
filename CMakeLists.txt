CMAKE_MINIMUM_REQUIRED(VERSION 3.9)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(OpenGL_GL_PREFERENCE "GLVND")
FIND_PACKAGE( OpenGL REQUIRED )

FIND_PACKAGE(glfw3 3.3 REQUIRED)

ADD_EXECUTABLE(
    sim
    src/main.cpp
    src/GLFWHandler.cpp
    src/glad.c
    src/SimpleFluid.cpp
    src/GLUtils.cpp
    src/SimulationFactory.cpp
    src/Smoke.cpp
)

TARGET_LINK_LIBRARIES(sim
    glfw
    OpenGL::GL
    ${CMAKE_DL_LIBS}
    )

ADD_CUSTOM_COMMAND(TARGET sim POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/src/kernels ${CMAKE_BINARY_DIR}/kernels
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/src/shaders ${CMAKE_BINARY_DIR}/shaders
  )
