#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>

class ImGuiManager {
public:
   ImGuiManager();

   // Commence une frame ImGui
   void newFrame();

   // Rendu ImGui
   void render();

   // Nettoyage
   void shutdown();
private:
   GLFWwindow* p_window;
   ImGuiIO* p_io;
};
