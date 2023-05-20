#pragma once

#include "glad/glad.h"

template<GLuint ShaderType>
class Shader
{
public:
    Shader(const char* source)
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
    ~Shader()
    {
        if(m_shader_id != 0)
            glDeleteShader(m_shader_id);
    }
    Shader(Shader&& other)
        : m_shader_id(other.m_shader_id)
    {
        other.m_shader_id = 0;
    }
    Shader& operator=(Shader&& other)
    {
        delete_shader();

        m_shader_id = other.m_shader_id;
        other.m_shader_id = 0;

        return *this;
    }

    void delete_shader()
    {
        if (m_shader_id != 0)
        {
            glDeleteShader(m_shader_id);
            m_shader_id = 0;
        }
    }

    bool valid() const { return m_shader_id != 0; }
    GLuint id() const { return m_shader_id; }
private:
    GLuint m_shader_id = 0;
};

class Program
{
public:
    Program() = default;
    Program(
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
    ~Program()
    {
        delete_program();
    }
    Program(Program&& other)
        : m_program_id(other.m_program_id)
    {
        other.m_program_id = 0;
    }
    Program& operator=(Program&& other)
    {
        delete_program();

        m_program_id = other.m_program_id;
        other.m_program_id = 0;

        return *this;
    }

    void delete_program()
    {
        if (m_program_id != 0)
        {
            glDeleteProgram(m_program_id);
            m_program_id = 0;
        }
    }
    void use() const
    {
        glUseProgram(m_program_id);
    }

    bool valid() const { return m_program_id != 0; }
    GLuint id() const { return m_program_id; }

private:
    GLuint m_program_id = 0;
};

class VertexArray
{
public:
    VertexArray() = default;
    VertexArray(float* vertices, int vertices_count, unsigned int* indices, int indices_count)
        : m_num_indices(indices_count)
    {
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices_count, vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &m_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices_count, indices, GL_STATIC_DRAW);

    }
    ~VertexArray()
    {
        delete_vertex_array();
    }
    VertexArray(VertexArray&& other)
        : m_vao(other.m_vao)
        , m_vbo(other.m_vbo)
        , m_ibo(other.m_ibo)
        , m_num_indices(other.m_num_indices)
    {
        other.m_vao = 0;
        other.m_vbo = 0;
        other.m_ibo = 0;
        other.m_num_indices = 0;
    }
    VertexArray& operator=(VertexArray&& other)
    {
        delete_vertex_array();

        m_vao = other.m_vao;
        m_vbo = other.m_vbo;
        m_ibo = other.m_ibo;
        m_num_indices = other.m_num_indices;

        other.m_vao = 0;
        other.m_vbo = 0;
        other.m_ibo = 0;
        other.m_num_indices = 0;

        return *this;
    }

    void delete_vertex_array()
    {
        if (m_vao != 0)
        {
            glDeleteVertexArrays(1, &m_vao);
            m_vao = 0;
        }
        if (m_vbo != 0)
        {
            glDeleteBuffers(1, &m_vbo);
            m_vbo = 0;
        }
        if (m_ibo != 0)
        {
            glDeleteBuffers(1, &m_ibo);
            m_ibo = 0;
        }
        m_num_indices = 0;
    }
    void use() const
    {
        glBindVertexArray(m_vao);
    }
    int num_indices() const { return m_num_indices; }

private:
    GLuint m_vao = 0;
    GLuint m_vbo = 0;   //could be multiple of these
    GLuint m_ibo = 0;
    int m_num_indices = 0;
};

inline void draw(const VertexArray& vao, const Program& shader_program)
{
    vao.use();
    shader_program.use();
    glDrawElements(GL_TRIANGLES, vao.num_indices(), GL_UNSIGNED_INT, nullptr);
}
