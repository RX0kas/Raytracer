

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

   // Used to render the raytraced image to a texture
   Shader shader("main.vert","main.frag");
   // Used to display the texture to the screen
   Shader screenShader("screenShader.vert","screenShader.frag");

   unsigned int VAO;
   gladManager::bindVAO(&VAO);

   printf("Initialising ImGui\n");
   ImGuiManager imGuiManager;

   printf("ImGui Initialized\n");

   float focalLength = 1;
   gladManager::setDeltaTime(0.0f);
   float lastFrame = 0.0f;
   int maxBounces = 20;

   Camera& camera = gladManager::getCamera();

   gladManager::generateFrameBuffer(window->width, window->height);

   unsigned int time = 0;
   int rayPerPixel = 50;

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
      ImGui::SliderInt("Ray per pixel",&rayPerPixel,1,100);
      ImGui::Separator();
      ImGui::Text("DeltaTime: %.2f",d);
      ImGui::Text("FPS: %.2f",1/d);
      ImGui::Separator();
      ImGui::Text("Uniforms:");
      ImGui::Text("focalLength = %.2f\n",focalLength);
      ImGui::Text("resolution = (%.2f,%.2f)\n",static_cast<float>(window->width), static_cast<float>(window->height));
      ImGui::Text("camDir = (%.2f,%.2f,%.2f)\n",dir.x,dir.y,dir.z);
      ImGui::Text("camUp = (%.2f,%.2f,%.2f)\n",up.x,up.y,up.z);
      ImGui::Text("camPos = (%.2f,%.2f,%.2f)\n",pos.x,pos.y,pos.z);
      ImGui::Text("time = %d",time);
      ImGui::Text("maxBounces = %d",maxBounces);
      ImGui::Text("lastMove = %d", gladManager::frameSinceLastMove);
      ImGui::Text("rayPerPixel = %d",rayPerPixel);
      ImGui::End();

      // Shader things
      if (moved) {
         gladManager::frameSinceLastMove = 0;
      } else {
         gladManager::frameSinceLastMove++;
      }

      int writeIndex = (gladManager::frameSinceLastMove % 2 == 0) ? 0 : 1;
      int readIndex = 1 - writeIndex; // on lit l’autre texture

      glBindFramebuffer(GL_FRAMEBUFFER, gladManager::framebuffers[writeIndex]);

      shader.useShader();
      shader.setFloat("focalLength", focalLength);
      shader.setVec2f("resolution", static_cast<float>(window->width), static_cast<float>(window->height));
      shader.setVec3f("camDir",dir.x,dir.y,dir.z);
      shader.setVec3f("camUp",up.x,up.y,up.z);
      shader.setVec3f("camPos",pos.x,pos.y,pos.z);
      shader.setUInt("time",time);
      shader.setInt("maxBounces",maxBounces);
      shader.setInt("lastMove", gladManager::frameSinceLastMove);
      shader.setInt("rayPerPixel",rayPerPixel);
      // Send old frame
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, gladManager::getAccumTexture());
      glUniform1i(glGetUniformLocation(shader.getProgram(), "oldFrame"), 0);

      // Render Triangle
      gladManager::draw();

      // Show the texture to the screen so the raytraced image
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      screenShader.useShader();
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, gladManager::textures[writeIndex]);

      gladManager::draw(); // affiche la texture sur l'écran

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