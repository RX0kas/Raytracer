#pragma once

#ifndef GLADMANAGER_HPP
#define GLADMANAGER_HPP

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <cassert>
#include <locale>
#include <memory>

#include "camera.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

class gladManager {
public:
   static void init(GLFWwindow* glfw_window) {
      // Charger OpenGL avec GLAD
      if (!gladLoadGL()) {
         fprintf(stderr, "Erreur: Impossible d'initialiser GLAD\n");
         glfwDestroyWindow(glfw_window);
         glfwTerminate();
         exit(EXIT_FAILURE);
      }

      const GLubyte* version = glGetString(GL_VERSION);
      const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

      printf("OpenGL version: %hhu\n",*version);
      printf("GLSL version: %hhu\n",*glslVersion);
   }

   static void bindVAO(unsigned int* VAO) {
      glGenVertexArrays(1, VAO);
      glBindVertexArray(*VAO);
   }

   static void unbindVAO(unsigned int* VAO) {
      glDeleteVertexArrays(1, VAO);
   }

   static void clear() {
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
   }

   static void draw() {
      glDrawArrays(GL_TRIANGLES, 0, 3);
   }

   static Camera& getCamera() {
      return *camera;
   }

   static void setDeltaTime(double deltaTime) {
      p_deltaTime = deltaTime;
   }

   static float getDeltaTime() {
      return p_deltaTime;
   }

private:
   static std::unique_ptr<Camera> camera;
   static float p_deltaTime;
};

#endif //GLADMANAGER_HPP