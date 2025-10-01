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

   static void generateFrameBuffer(int width, int height) {
      glGenFramebuffers(2, framebuffers);
      glGenTextures(2, textures);

      for (int i = 0; i < 2; i++) {
         glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i]);

         glBindTexture(GL_TEXTURE_2D, textures[i]);
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[i], 0);

         if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer " << i << " not complete!" << std::endl;
         }
      }

      glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind
   }

   static void renderingToTexture(int width, int height) {
      int index = writeToFirstTexture ? 0 : 1;
      glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[index]);
      glViewport(0, 0, width, height);

      // alterne pour la prochaine frame
      writeToFirstTexture = !writeToFirstTexture;
   }

   static GLuint getAccumTexture() {
      // renvoie la texture "ancienne" (celle qu’on ne vient pas de remplir)
      return textures[writeToFirstTexture ? 1 : 0];
   }

public:
   static GLuint framebuffers[2];
   static GLuint textures[2];
   static bool writeToFirstTexture;
private:
   static std::unique_ptr<Camera> camera;
   static float p_deltaTime;
};

#endif //GLADMANAGER_HPP