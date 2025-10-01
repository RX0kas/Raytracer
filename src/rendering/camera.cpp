#include "camera.hpp"

#include "rendering/gladManager.hpp"

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
   gladManager::frameSinceLastMove = 0;

   xoffset *= MouseSensitivity;
   yoffset *= MouseSensitivity;

   Yaw   += xoffset;
   Pitch += yoffset;

   // make sure that when pitch is out of bounds, screen doesn't get flipped
   if (constrainPitch)
   {
      if (Pitch > 89.0f)
         Pitch = 89.0f;
      if (Pitch < -89.0f)
         Pitch = -89.0f;
   }

   // update Front, Right and Up Vectors using the updated Euler angles
   updateCameraVectors();
}