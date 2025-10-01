cmake_minimum_required(VERSION 4.0.2)

project(Raytracer C CXX)

# Standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# GLFW
find_package(glfw3 3.4 QUIET)
if (NOT glfw3_FOUND)
    FetchContent_Declare(
            glfw3
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip
    )
    FetchContent_GetProperties(glfw3)
    if (NOT glfw3_POPULATED)
        set(FETCHCONTENT_QUIET NO)
        FetchContent_MakeAvailable(glfw3)
        #add_subdirectory(${glfw3_SOURCE_DIR} ${glfw3_BINARY_DIR})
    endif()
endif()

# OpenGL
find_package(OpenGL REQUIRED)

# --- GLAD ---
#FetchContent_Declare(
#        glad
#        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
#        GIT_TAG v2.0.8
#)
#FetchContent_MakeAvailable(glad)
#
#add_subdirectory(${glad_SOURCE_DIR}/cmake glad_cmake)
#glad_add_library(glad REPRODUCIBLE LOADER API gl:core=4.6)

# --- ImGui ---
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG docking   # plus complet que master
)
FetchContent_MakeAvailable(imgui)

add_library(imgui_glfw_opengl3
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui_glfw_opengl3 PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui_glfw_opengl3 PUBLIC glfw OpenGL::GL)

# GLM
FetchContent_Declare(
        glm
        GIT_REPOSITORY	https://github.com/g-truc/glm.git
        GIT_TAG 	bf71a834948186f4097caa076cd2663c69a10e1e #refs/tags/1.0.1
)

FetchContent_MakeAvailable(glm)

# --- Exécutable ---
add_executable(Raytracer
        src/main.cpp
        src/window.cpp src/window.hpp
        src/imgui/imGuiManager.cpp src/imgui/imGuiManager.hpp
        src/rendering/gladManager.hpp
        src/glad/glad.c src/glad/glad.h src/glad/khrplatform.h
        src/rendering/shader.cpp src/rendering/shader.hpp
        src/rendering/Camera.hpp
        src/rendering/gladManager.cpp
)

# --- Liens ---
target_link_libraries(Raytracer
        PRIVATE
        #glad
        glfw
        OpenGL::GL
        imgui_glfw_opengl3
        glm::glm
)

target_include_directories(Raytracer PRIVATE
        src
)
