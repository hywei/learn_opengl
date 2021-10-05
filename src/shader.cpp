#include "shader.h"
#include <fstream>
#include <glad/glad.h>
#include <iostream>
#include <sstream>

Shader::Shader(const char* vs_path, const char* fs_path, const char* gs_path)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertex_code;
    std::string fragment_code;
    std::string geometry_code;

    std::ifstream v_shader_file;
    std::ifstream f_shader_file;
    std::ifstream g_shader_file;

    v_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    f_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    g_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        v_shader_file.open(vs_path);
        f_shader_file.open(fs_path);

        std::stringstream v_shader_stream, f_shader_stream;

        v_shader_stream << v_shader_file.rdbuf();
        f_shader_stream << f_shader_file.rdbuf();

        v_shader_file.close();
        f_shader_file.close();

        // convert stream to string
        vertex_code   = v_shader_stream.str();
        fragment_code = f_shader_stream.str();

        // if geometry shader path is present, also load a geometry shader
        if (gs_path != nullptr)
        {
            g_shader_file.open(gs_path);
            std::stringstream g_shader_stream;
            g_shader_stream << g_shader_file.rdbuf();
            g_shader_file.close();
            geometry_code = g_shader_stream.str();
        }
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_READ_FAILED" << std::endl;
    }

    const char* vs_code = vertex_code.c_str();
    const char* fs_code = fragment_code.c_str();

    // 2. compile shaders
    int  success;
    char info_log[512];

    uint32_t vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vs_code, nullptr);
    glCompileShader(vertex_shader);
    checkCompileErrors(vertex_shader, "VERTEX");

    uint32_t fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fs_code, nullptr);
    glCompileShader(fragment_shader);
    checkCompileErrors(fragment_shader, "FRAGMENT");

    uint32_t geometry_shader = 0;
    if (gs_path != nullptr)
    {
        const char* gs_code = geometry_code.c_str();
        geometry_shader     = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry_shader, 1, &gs_code, nullptr);
        glCompileShader(geometry_shader);
        checkCompileErrors(geometry_shader, "GEOMETRY");
    }

    ID = glCreateProgram();
    glAttachShader(ID, vertex_shader);
    glAttachShader(ID, fragment_shader);
    if (gs_path != nullptr)
    {
        glAttachShader(ID, geometry_shader);
    }
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    // delete the shaders as they're linked into our pragram now
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    if (gs_path != nullptr)
    {
        glDeleteShader(geometry_shader);
    }
}

void Shader::use()
{
    glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec3f(const std::string& name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::setVec4f(const std::string& name, float x, float y, float z, float w) const
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

void Shader::setMat4fv(const std::string& name, const float* values) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, values);
}

void Shader::checkCompileErrors(uint32_t shader, std::string type)
{
    GLint  success;
    GLchar info_log[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, nullptr, info_log);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                      << info_log << "\n -- --------------------------------------------------- -- "
                      << std::endl;
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, nullptr, info_log);
                std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                          << info_log
                          << "\n -- --------------------------------------------------- -- "
                          << std::endl;
            }
        }
    }
}