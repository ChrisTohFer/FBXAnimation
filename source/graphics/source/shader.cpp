#include "graphics/shader.h"

namespace graphics
{

    Program::~Program()
    {
        delete_program();
    }

    Program::Program(
        const Shader<GL_VERTEX_SHADER>& vertex_shader,
        const Shader<GL_FRAGMENT_SHADER>& fragment_shader)
    {
        int  success;
        char info_log[512];

        m_program_id = glCreateProgram();
        glAttachShader(m_program_id, vertex_shader.id());
        glAttachShader(m_program_id, fragment_shader.id());
        glLinkProgram(m_program_id);
        glGetProgramiv(m_program_id, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(m_program_id, 512, nullptr, info_log);
            std::cout << info_log << "\n";
        }
    }
    Program::Program(Program&& other)
        : m_program_id(other.m_program_id)
    {
        other.m_program_id = 0;
    }
    Program& Program::operator=(Program&& other)
    {
        delete_program();

        m_program_id = other.m_program_id;
        other.m_program_id = 0;

        return *this;
    }

    void Program::delete_program()
    {
        if (m_program_id != 0)
        {
            glDeleteProgram(m_program_id);
            m_program_id = 0;
        }
    }
    void Program::use() const
    {
        glUseProgram(m_program_id);
    }

    void Program::set_uniform(const char* name, const Colour& colour) const
    {
        unsigned int location = glGetUniformLocation(m_program_id, name);
        glUniform4f(location, colour.r, colour.g, colour.b, colour.a);
    }

    void Program::set_uniform(const char* name, const geom::Matrix44& matrix) const
    {
        unsigned int location = glGetUniformLocation(m_program_id, name);
        glUniformMatrix4fv(location, 1, GL_FALSE, matrix.values);
    }

    void Program::set_uniform(const char* name, const std::vector<geom::Matrix44>& matrices) const
    {
        unsigned int location = glGetUniformLocation(m_program_id, name);
        glUniformMatrix4fv(location, (int)matrices.size(), GL_FALSE, matrices[0].values);
    }

}