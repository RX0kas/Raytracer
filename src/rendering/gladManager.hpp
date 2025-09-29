#pragma once

#ifndef GLADMANAGER_HPP
#define GLADMANAGER_HPP

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <cassert>
#include <iostream>
#include <locale>
#include <memory>
#include <vector>

#include "camera.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

struct Sphere {
   glm::vec3 center;
   float radius;
};

constexpr int NBR_SPHERES = 3;

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

   static void createUBO(unsigned int shaderProgram, const std::vector<Sphere>& spheres) {
      // Create and bind the UBO
      GLuint uboSphere;
      glGenBuffers(1, &uboSphere);
      glBindBuffer(GL_UNIFORM_BUFFER, uboSphere);
      glBufferData(GL_UNIFORM_BUFFER, sizeof(Sphere) * NBR_SPHERES, NULL, GL_DYNAMIC_DRAW);

      // Bind the UBO to a specific binding point
      GLuint uniformBlockIndex = glGetUniformBlockIndex(shaderProgram, "LightBlock");
      glUniformBlockBinding(shaderProgram, uniformBlockIndex, 0);
      glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboSphere);

      // Update the data in the UBO
      glBindBuffer(GL_UNIFORM_BUFFER, uboSphere);
      glBufferSubData(GL_UNIFORM_BUFFER, 0, spheres.size(), spheres.data());
      glBindBuffer(GL_UNIFORM_BUFFER, 0);
   }

   static void createAccumulationTexture(int width, int height) {
      // create the texture
      GLuint accumTex;
      glGenTextures(1, &accumTex);
      glBindTexture(GL_TEXTURE_2D, accumTex);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

      // Filtres = nearest, pas de mipmaps
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


      // Attached the texture
      GLuint accumFBO;
      glGenFramebuffers(1, &accumFBO);
      glBindFramebuffer(GL_FRAMEBUFFER, accumFBO);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumTex, 0);

      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
         std::cerr << "Framebuffer not complete!" << std::endl;
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
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