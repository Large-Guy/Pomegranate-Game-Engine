#ifndef SHADERS_H
#define SHADERS_H
#include"string"
#include "../lib/glad/glad.h"
#include"GLFW/glfw3.h"
#include"glm.hpp"
#include"ext.hpp"
#include "sstream"
#include"fstream"
#include"vector"
#include"iostream"
unsigned int make_module(const std::string& filepath, unsigned int module_type);
unsigned int make_shader(const std::string& vertex_filepath, const std::string& fragment_filepath);
void set_shader_vector4_value(GLuint shaderProgram, const char* name,glm::vec4 value);
void set_shader_matrix4_value(GLuint shaderProgram, const char* name,glm::mat4 value);
class Material
{
public:
    unsigned int GL_shader_id;
    Material(std::string shader_path);
    Material();
};

#endif