#include "gladManager.hpp"

std::unique_ptr<Camera> gladManager::camera = std::make_unique<Camera>();
float gladManager::p_deltaTime = 0.0f;