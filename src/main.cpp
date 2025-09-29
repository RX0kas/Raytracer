

#include <cstdio>
#include <cstdlib>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "window.hpp"
#include "imgui/imGuiManager.hpp"
#include "rendering/camera.hpp"
#include "rendering/gladManager.hpp"
#include "rendering/shader.hpp"

Window* window;

std::vector<Sphere> spheres = {
   Sphere{
      .center = glm::vec3(0, 0, 10),
      .radius = 2
   },
   Sphere{
      .center = glm::vec3(10, 0, 10),
      .radius = 1
   },
   Sphere{
      .center = glm::vec3(10, 0, 10),
      .radius = 1
   }
};

int main() {
   printf("Initializing GLFW\n");
   window = windowInit("RayTracer",800,600);
   glfwPollEvents();
   printf("GLFW initialized\n");

   Shader shader("main.vert","main.frag");

   unsigned int VAO;
   gladManager::bindVAO(&VAO);

   printf("Initialising ImGui\n");
   ImGuiManager imGuiManager;

   printf("ImGui Initialized\n");

   //printf("Bind UBO");
   //gladManager::createUBO(shader.getProgram(),spheres);

   float focalLength = 1;
   gladManager::setDeltaTime(0.0f);
   float lastFrame = 0.0f;
   int maxBounces = 20;

   Camera& camera = gladManager::getCamera();

   gladManager::createAccumulationTexture(window->width, window->height);

   unsigned int time = 0;
   int frameSinceLastMove = 0;

   // Boucle principale
   while (!windowShouldClose()) {
      auto currentFrame = static_cast<float>(glfwGetTime());
      gladManager::setDeltaTime(currentFrame - lastFrame);
      float d = gladManager::getDeltaTime();
      lastFrame = currentFrame;

      bool moved = false;

      // Input
      if (glfwGetKey(window->window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
         glfwSetWindowShouldClose(window->window, true);

      if (glfwGetKey(window->window, GLFW_KEY_N) == GLFW_PRESS) {
         changeMouseMode(window->window);
      }

      if (glfwGetKey(window->window, GLFW_KEY_W) == GLFW_PRESS) {
         camera.ProcessKeyboard(FORWARD, d);
         moved = true;
      }
      if (glfwGetKey(window->window, GLFW_KEY_S) == GLFW_PRESS) {
         camera.ProcessKeyboard(BACKWARD, d);
         moved = true;
      }
      if (glfwGetKey(window->window, GLFW_KEY_A) == GLFW_PRESS) {
         camera.ProcessKeyboard(LEFT, d);
         moved = true;
      }
      if (glfwGetKey(window->window, GLFW_KEY_D) == GLFW_PRESS) {
         camera.ProcessKeyboard(RIGHT, d);
         moved = true;
      }

      imGuiManager.newFrame();
      // Couleur de fond
      gladManager::clear();

      glm::vec3 dir = camera.Front;
      glm::vec3 up = camera.Up;

      // ImGui Widget
      ImGui::Begin("Camera");
      ImGui::SliderFloat("Focal length",&focalLength,0,10,"%.1f");
      glm::vec3 pos = gladManager::getCamera().Position;
      glm::vec3 front = gladManager::getCamera().Front;
      ImGui::Text("Position: %.2f,%.2f,%.2f",pos.x,pos.y,pos.z);
      ImGui::Text("Front: %.2f,%.2f,%.2f",front.x,front.y,front.z);
      ImGui::Separator();
      ImGui::SliderInt("Max bounces",&maxBounces,2,100);
      ImGui::Separator();
      ImGui::Text("DeltaTime: %.2f",d);
      ImGui::Text("FPS: %.2f",1/d);
      ImGui::Separator();
      ImGui::Text("Uniforms:");
      ImGui::Text("focal_length = %.2f\n",focalLength);
      ImGui::Text("resolution = (%.2f,%.2f)\n",static_cast<float>(window->width), static_cast<float>(window->height));
      ImGui::Text("camdir = (%.2f,%.2f,%.2f)\n",dir.x,dir.y,dir.z);
      ImGui::Text("camup = (%.2f,%.2f,%.2f)\n",up.x,up.y,up.z);
      ImGui::Text("campos = (%.2f,%.2f,%.2f)\n",pos.x,pos.y,pos.z);
      ImGui::Text("time = %d",time);
      ImGui::Text("maxbounces = %d",maxBounces);
      ImGui::End();

      // Shader things
      if (moved) {
         frameSinceLastMove = 0;
      } else {
         frameSinceLastMove++;
      }

      shader.useShader();
      shader.setFloat("focal_length", focalLength);
      shader.setVec2f("resolution", static_cast<float>(window->width), static_cast<float>(window->height));
      shader.setVec3f("camdir",dir.x,dir.y,dir.z);
      shader.setVec3f("camup",up.x,up.y,up.z);
      shader.setVec3f("campos",pos.x,pos.y,pos.z);
      shader.setUInt("time",time);
      shader.setInt("maxbounces",maxBounces);
      shader.setInt("lastmove",frameSinceLastMove);

      // Render Triangle
      gladManager::draw();

      imGuiManager.render();

      glfwSwapBuffers(window->window);
      glfwPollEvents();
      time++;

   }

   gladManager::unbindVAO(&VAO);

   // Nettoyer
   imGuiManager.shutdown();
   windowClose();

   return EXIT_SUCCESS;
}