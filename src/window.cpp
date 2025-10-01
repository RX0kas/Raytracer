#include "window.hpp"

#include <cstdio>

#include "rendering/gladManager.hpp"

static int windowInitialized = 0;
static Window window;
static Camera& camera = gladManager::getCamera();
bool firstMouse = true;
float lastX = 800 / 2.0f;
float lastY = 600 / 2.0f;

Window* windowInstance() {
   return &window;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
   glViewport(0, 0, width, height);
   windowInstance()->width = width;
   windowInstance()->height = height;
   gladManager::regenerateFrameBuffer(width, height);
   gladManager::frameSinceLastMove = 0;
}

static bool mouseEnabled = true;

void changeMouseMode(GLFWwindow* w) {
   mouseEnabled = !mouseEnabled;
   if (mouseEnabled) {
      glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
   } else {
      glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
   }
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
   if (mouseEnabled) return;
   float xpos = static_cast<float>(xposIn);
   float ypos = static_cast<float>(yposIn);

   if (firstMouse)
   {
      lastX = xpos;
      lastY = ypos;
      firstMouse = false;
   }

   float xoffset = xpos - lastX;
   float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

   lastX = xpos;
   lastY = ypos;

   camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
   camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

Window* windowInit(const std::string& title,int width,int height) {
   if (windowInitialized) {
      fprintf(stderr, "Window already initialized\n");
      exit(EXIT_FAILURE);
   }
   windowInitialized = 1;

   // Initialiser GLFW
   if (!glfwInit()) {
      fprintf(stderr, "Erreur: Impossible d'initialiser GLFW\n");
      exit(EXIT_FAILURE);
   }


   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

   // Créer une fenêtre
   GLFWwindow* glfw_window = glfwCreateWindow(width, height, title.c_str(), nullptr,  nullptr);

   if (!glfw_window) {
      fprintf(stderr, "Erreur: Impossible de créer la fenêtre GLFW\n");
      glfwTerminate();
      exit(EXIT_FAILURE);
   }
   glfwMakeContextCurrent(glfw_window);

   glfwSwapInterval(1); // Enable vsync

   gladManager::init(glfw_window);

   // Définir le callback pour le redimensionnement
   glfwSetFramebufferSizeCallback(glfw_window, framebuffer_size_callback);
   glfwSetCursorPosCallback(glfw_window, mouse_callback);
   glfwSetScrollCallback(glfw_window, scroll_callback);

   // tell GLFW to capture our mouse
   //glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

   window.window = glfw_window;
   window.title = title;
   window.height = height;
   window.width = width;

   return &window;
}

int windowShouldClose() {
   return glfwWindowShouldClose(window.window);
}

void windowClose() {
   glfwDestroyWindow(window.window);
   glfwTerminate();
}