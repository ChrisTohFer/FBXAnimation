#pragma once

#include "colour.h"

#include "maths/geometry.h"

#include "glad/glad.h"

#include <iostream>
#include <vector>

namespace graphics
{

    template<unsigned int ShaderType>
    class Shader
    {
    public:
        ~Shader();
        Shader(const char* source);
        Shader(Shader&& other);
        Shader& operator=(Shader&& other);

        void delete_shader();

        bool valid() const { return m_shader_id != 0; }
        unsigned int id() const { return m_shader_id; }
    private:
        unsigned int m_shader_id = 0;
    };

    class Program
    {
    public:
        ~Program();
        Program(
            const Shader<GL_VERTEX_SHADER>& vertex_shader,
            const Shader<GL_FRAGMENT_SHADER>& fragment_shader);
        Program(Program&& other);

        Program& operator=(Program&& other);

        void delete_program();
        void use() const;

        void set_uniform(const char* name, const Colour& colour) const;
        void set_uniform(const char* name, const geom::Matrix44& matrix) const;
        void set_uniform(const char* name, const std::vector<geom::Matrix44>& matrices) const;

        bool valid() const { return m_program_id != 0; }
        unsigned int id() const { return m_program_id; }

    private:
        unsigned int m_program_id = 0;
    };

    //inline definitions

    //shader

    template<unsigned int ShaderType>
    Shader<ShaderType>::Shader(const char* source)
    {
        int  success;
        char info_log[512];

        m_shader_id = glCreateShader(ShaderType);
        glShaderSource(m_shader_id, 1, &source, nullptr);
        glCompileShader(m_shader_id);
        glGetShaderiv(m_shader_id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(m_shader_id, 512, nullptr, info_log);
            std::cout << "Error in shader. Source:\n" << source << "\nError:" << info_log << "\n";
        }
    }

    template<unsigned int ShaderType>
    Shader<ShaderType>::~Shader()
    {
        if (m_shader_id != 0)
            glDeleteShader(m_shader_id);
    }

    template<unsigned int ShaderType>
    Shader<ShaderType>::Shader(Shader&& other)
        : m_shader_id(other.m_shader_id)
    {
        other.m_shader_id = 0;
    }

    template<unsigned int ShaderType>
    Shader<ShaderType>& Shader<ShaderType>::operator=(Shader&& other)
    {
        delete_shader();

        m_shader_id = other.m_shader_id;
        other.m_shader_id = 0;

        return *this;
    }

    template<unsigned int ShaderType>
    void Shader<ShaderType>::delete_shader()
    {
        if (m_shader_id != 0)
        {
            glDeleteShader(m_shader_id);
            m_shader_id = 0;
        }
    }
}