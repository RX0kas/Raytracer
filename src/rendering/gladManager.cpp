#include "gladManager.hpp"

std::unique_ptr<Camera> gladManager::camera = std::make_unique<Camera>();
float gladManager::p_deltaTime = 0.0f;
bool gladManager::writeToFirstTexture = true;
GLuint gladManager::framebuffers[2] = {0,1};
GLuint gladManager::textures[2] = {0,1};