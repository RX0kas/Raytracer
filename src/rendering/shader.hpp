#pragma once

#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>


class Shader {
public:
   Shader(const std::string &vertexShaderPath, const std::string &fragmentShaderPath);

   void useShader() const;
   void setBool(const std::string& name, int value) const;
   void setInt(const std::string& name, int value) const;
   void setFloat(const std::string& name, float value) const;
   void setVec2f(const std::string& name, float v0, float v1) const;
   void setVec3f(const std::string& name, float v0, float v1, float v2) const;
private:
   unsigned int program;
};



#endif //SHADER_HPP
