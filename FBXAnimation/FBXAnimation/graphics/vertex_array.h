#pragma once

#include "vertex_buffer.h"

#include "glad/glad.h"

namespace graphics
{

    //vertex arrays
    template<Vertex VertexType>
    class VertexArray
    {
    public:
        ~VertexArray();
        VertexArray(VertexBuffer<VertexType> vertices, unsigned int* indices, int indices_count);
        VertexArray(VertexArray&& other);
        VertexArray& operator=(VertexArray&& other);

        void delete_vertex_array();
        void use() const;
        int num_indices() const;

    private:
        VertexBuffer<VertexType> m_vbo;
        unsigned int m_vao = 0;
        unsigned int m_ibo = 0;
        int m_num_indices = 0;
    };

    //inline definitions

    template<Vertex VertexType>
    VertexArray<VertexType>::~VertexArray()
    {
        delete_vertex_array();
    }

    template<Vertex VertexType>
    VertexArray<VertexType>::VertexArray(VertexBuffer<VertexType> vertices, unsigned int* indices, int indices_count)
        : m_num_indices(indices_count)
        , m_vbo(std::move(vertices))
    {
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        m_vbo.bind();

        glGenBuffers(1, &m_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices_count, indices, GL_STATIC_DRAW);
    }

    template<Vertex VertexType>
    VertexArray<VertexType>::VertexArray(VertexArray<VertexType>&& other)
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

    template<Vertex VertexType>
    VertexArray<VertexType>& VertexArray<VertexType>::operator=(VertexArray<VertexType>&& other)
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

    template<Vertex VertexType>
    void VertexArray<VertexType>::delete_vertex_array()
    {
        if (m_vao != 0)
        {
            glDeleteVertexArrays(1, &m_vao);
            m_vao = 0;
        }
        if (m_ibo != 0)
        {
            glDeleteBuffers(1, &m_ibo);
            m_ibo = 0;
        }
        m_num_indices = 0;
    }

    template<Vertex VertexType>
    void VertexArray<VertexType>::use() const
    {
        glBindVertexArray(m_vao);
    }

    template<Vertex VertexType>
    int VertexArray<VertexType>::num_indices() const
    {
        return m_num_indices;
    }

}