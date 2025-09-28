#include "shader.hpp"

#include <fstream>
#include <iostream>

#include <glad/glad.h>


std::string read_file(const std::string& filepath) {
   std::ifstream file(filepath, std::ios::in | std::ios::binary | std::ios::ate);
   if (!file) {
      throw std::runtime_error("Unable to open file: " + filepath);
   }

   std::streamsize size = file.tellg();
   file.seekg(0, std::ios::beg);

   std::string buffer(size, '\0'); // alloue directement la bonne taille
   if (!file.read(buffer.data(), size)) {
      throw std::runtime_error("Error reading file: " + filepath);
   }

   return buffer;
}

Shader::Shader(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) {
   int success;
   char infoLog[512];

   program = glCreateProgram();

   // Lire les fichiers shaders
   std::string contentV = read_file(vertexShaderPath);
   const char* srcV = contentV.c_str();
   std::string contentF = read_file(fragmentShaderPath);
   const char* srcF = contentF.c_str();

   if (contentV.empty() || contentF.empty()) {
      fprintf(stderr, "Erreur lecture shader files\n");
      exit(1);
   }

   // Vertex Shader
   unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(vertexShader, 1, &srcV, nullptr);
   glCompileShader(vertexShader);

   glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
   if(!success) {
      glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
      printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
   }

   // Fragment Shader
   unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
   glShaderSource(fragmentShader, 1, &srcF, nullptr);
   glCompileShader(fragmentShader);

   glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
   if(!success) {
      glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
      printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
   }

   // Link
   glAttachShader(program, vertexShader);
   glAttachShader(program, fragmentShader);
   glLinkProgram(program);

   glGetProgramiv(program, GL_LINK_STATUS, &success);
   if(!success) {
      glGetProgramInfoLog(program, 512, nullptr, infoLog);
      printf("ERROR::SHADER::PROGRAM::LINK_FAILED\n%s\n", infoLog);
      program = 0;
   }

   // Nettoyage
   glDeleteShader(vertexShader);
   glDeleteShader(fragmentShader);
}


void Shader::useShader() const {
   glUseProgram(program);
}
void Shader::setBool(const std::string& name, int value) const {
   glUniform1i(glGetUniformLocation(program, name.c_str()), value);
}
void Shader::setInt(const std::string& name, int value) const {
   glUniform1i(glGetUniformLocation(program, name.c_str()), value);
}
void Shader::setFloat(const std::string& name, float value) const {
   glUniform1f(glGetUniformLocation(program, name.c_str()), value);
}
void Shader::setVec2f(const std::string& name, float v0, float v1) const {
   glUniform2f(glGetUniformLocation(program, name.c_str()), v0, v1);
}
void Shader::setVec3f(const std::string& name, float v0, float v1, float v2)  const {
   glUniform3f(glGetUniformLocation(program, name.c_str()), v0, v1, v2);
}