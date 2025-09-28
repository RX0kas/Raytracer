#include "imGuiManager.hpp"

#include "window.hpp"

ImGuiManager::ImGuiManager() : p_window(windowInstance()->window) {
   static auto glsl_version = "#version 330 core";

   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO& io = ImGui::GetIO(); (void) io;
   p_io = &io;
   io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
   io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
   io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
   io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
   //io.ConfigViewportsNoAutoMerge = true;
   //io.ConfigViewportsNoTaskBarIcon = true;

   ImGui::StyleColorsDark();
   //ImGui::StyleColorsLight();

   // Setup scaling
   ImGuiStyle &style = ImGui::GetStyle();
   style.ScaleAllSizes(ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()));
   // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
   //style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

   // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
   if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      style.WindowRounding = 0.0f;
      style.Colors[ImGuiCol_WindowBg].w = 1.0f;
   }

   ImGui_ImplGlfw_InitForOpenGL(p_window, true);
   ImGui_ImplOpenGL3_Init(glsl_version);
}


void ImGuiManager::newFrame()
{
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();
}

void ImGuiManager::render()
{

   ImGui::Render();
   //int display_w, display_h;
   //glfwGetFramebufferSize(p_window, &display_w, &display_h);
   //glViewport(0, 0, display_w, display_h);
   //glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
   //glClear(GL_COLOR_BUFFER_BIT);
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

   // Update and Render additional Platform Windows
   // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
   //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
   if (p_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
   {
      GLFWwindow* backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
   }
}

void ImGuiManager::shutdown()
{
   ImGui_ImplOpenGL3_Shutdown();
   ImGui_ImplGlfw_Shutdown();
   ImGui::DestroyContext();
}
