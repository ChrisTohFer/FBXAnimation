#pragma once

#include "glad/glad.h"

#include <vector>

namespace graphics
{

    //vertex buffers
    template<typename T>
    concept Vertex = alignof(T) == 4 && requires() { T::apply_attributes(); };

    template<Vertex VertexType>
    class VertexBuffer
    {
    public:
        ~VertexBuffer();
        VertexBuffer(const std::vector<VertexType>& vertices);
        VertexBuffer(VertexBuffer&& other);
        VertexBuffer& operator=(VertexBuffer&& other);

        void bind() const;
    private:
        unsigned int m_vbo = 0;
    };

    //inline definitions

    template<Vertex VertexType>
    VertexBuffer<VertexType>::~VertexBuffer()
    {
        if (m_vbo != 0)
        {
            glDeleteBuffers(1, &m_vbo);
            m_vbo = 0;
        }
    }

    template<Vertex VertexType>
    VertexBuffer<VertexType>::VertexBuffer(const std::vector<VertexType>& vertices)
    {
        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(VertexType) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    }

    template<Vertex VertexType>
    VertexBuffer<VertexType>::VertexBuffer(VertexBuffer&& other)
        : m_vbo(other.m_vbo)
    {
        other.m_vbo = 0;
    }

    template<Vertex VertexType>
    VertexBuffer<VertexType>& VertexBuffer<VertexType>::operator=(VertexBuffer<VertexType>&& other)
    {
        m_vbo = other.m_vbo;
        other.m_vbo = 0;
        return *this;
    }

    template<Vertex VertexType>
    void VertexBuffer<VertexType>::bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        VertexType::apply_attributes();
    }
}