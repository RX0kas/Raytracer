#pragma once

#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>

struct Window {
   GLFWwindow* window;
   std::string title;
   int width, height;
};

struct Window* windowInit(const std::string& title,int width,int height);

int windowShouldClose();

void windowClose();

struct Window* windowInstance();

void changeMouseMode(GLFWwindow* w);

#endif //WINDOW_H
