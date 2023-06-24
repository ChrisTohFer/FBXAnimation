#pragma once

#include "camera.h"
#include "colour.h"
#include "shader.h"
#include "vertex_array.h"


namespace graphics
{
    template<Vertex VType>
    class UnskinnedMeshShader : public Program
    {
    public:
        UnskinnedMeshShader();

        void draw(const VertexArray<VType>& vao, const geom::Matrix44& camera, const geom::Matrix44& world);
    };

    template<Vertex VType>
    class SkinnedMeshShader : public Program
    {
    public:
        SkinnedMeshShader();

        void draw(
            const VertexArray<VType>& vao,
            const geom::Matrix44& camera,
            const geom::Matrix44& world,
            const std::vector<geom::Matrix44>& pose_matrix_stack,
            const std::vector<geom::Matrix44>& inverse_matrix_stack);
    };

    class DebugShader : public Program
    {
    public:
        DebugShader();

        void draw_point(
            const Camera& camera,
            const geom::Vector3& point,
            float size = 0.02f,
            Colour colour = Colour::red());

        void draw_line(
            const Camera& camera,
            const geom::Vector3& p1,
            const geom::Vector3& p2,
            float thickness = 0.02f,
            Colour colour = Colour::red());

    private:
        //create line vertices
        struct VectorVertex
        {
            geom::Vector3 pos;

            static void apply_attributes()
            {
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VectorVertex), (void*)0);
                glEnableVertexAttribArray(0);
            }
        };

        void draw(const geom::Matrix44& camera, const std::vector<VectorVertex>& vertices, Colour colour);
    };


    //inline definitions

    //unskinned mesh

    const char* unskinned_mesh_vertex_shader =
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPos;"

        "uniform mat4 camera;"
        "uniform mat4 world;"

        "void main()"
        "{"
        "gl_Position = camera * world * vec4(aPos.x, aPos.y, aPos.z, 1.0);"
        "};";
    const char* unskinned_mesh_fragment_shader =
        "#version 330 core\n"
        "out vec4 FragColor;"

        "void main()"
        "{"
        "FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);"
        "}";
    template<Vertex VType>
    UnskinnedMeshShader<VType>::UnskinnedMeshShader()
        : Program(unskinned_mesh_vertex_shader, unskinned_mesh_fragment_shader)
    {}

    template<Vertex VType>
    void UnskinnedMeshShader<VType>::draw(const VertexArray<VType>& vao, const geom::Matrix44& camera, const geom::Matrix44& world)
    {
        use();
        set_uniform("camera", camera);
        set_uniform("world", world);
        vao.use();

        glDrawElements(GL_TRIANGLES, vao.num_indices(), GL_UNSIGNED_INT, nullptr);
    }

    //skinned mesh

    const char* skinned_mesh_vertex_shader =
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPos;"
        "layout(location = 1) in int bone;"

        "uniform mat4 camera;"
        "uniform mat4 world;"
        "uniform mat4 bones[50];"
        "uniform mat4 inv_bones[50];"

        "void main()"
        "{"
        "mat4 bone_matrix = bones[bone];"
        "mat4 inv_bone_matrix = inv_bones[bone];"
        "gl_Position = camera * world * bone_matrix * inv_bone_matrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);"
        "};";
    const char* skinned_mesh_fragment_shader =
        "#version 330 core\n"
        "out vec4 FragColor;"

        "void main()"
        "{"
        "FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);"
        "}";
    template<Vertex VType>
    SkinnedMeshShader<VType>::SkinnedMeshShader()
        : Program(skinned_mesh_vertex_shader, skinned_mesh_fragment_shader)
    {}

    template<Vertex VType>
    void SkinnedMeshShader<VType>::draw(
        const VertexArray<VType>& vao,
        const geom::Matrix44& camera,
        const geom::Matrix44& world,
        const std::vector<geom::Matrix44>& pose_matrix_stack,
        const std::vector<geom::Matrix44>& inverse_matrix_stack)
    {
        use();
        set_uniform("camera", camera);
        set_uniform("world", world);
        set_uniform("bones", pose_matrix_stack);
        set_uniform("inv_bones", inverse_matrix_stack);
        vao.use();

        glDrawElements(GL_TRIANGLES, vao.num_indices(), GL_UNSIGNED_INT, nullptr);
    }

    //line

    const char* line_vertex_shader =
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPos;"

        "uniform mat4 camera;"
        "uniform vec4 colour;"
        
        "out vec4 colour_internal;"

        "void main()"
        "{"
        "colour_internal = colour;"
        "gl_Position = camera * vec4(aPos.x, aPos.y, aPos.z, 1.0);"
        "};";
    const char* line_fragment_shader =
        "#version 330 core\n"
        "in vec4 colour_internal;"
        "out vec4 FragColor;"

        "void main()"
        "{"
        "FragColor = colour_internal;"
        "}";
    DebugShader::DebugShader()
        : Program(line_vertex_shader, line_fragment_shader)
    {}

    void DebugShader::draw_point(
        const Camera& camera,
        const geom::Vector3& point,
        float size,
        Colour colour)
    {
        geom::Vector3 x_offset = 0.5f * size * geom::Vector3::cross(point - camera.translation, geom::Vector3::unit_y()).normalized();
        geom::Vector3 y_offset = 0.5f * size * geom::Vector3::cross(point - camera.translation, x_offset).normalized();
        std::vector<VectorVertex> vertices;
        vertices.reserve(6);
        vertices.push_back({ point - x_offset - y_offset });
        vertices.push_back({ point - x_offset + y_offset });
        vertices.push_back({ point + x_offset + y_offset });
        vertices.push_back({ point - x_offset - y_offset });
        vertices.push_back({ point + x_offset + y_offset });
        vertices.push_back({ point + x_offset - y_offset });

        draw(camera.calculate_camera_matrix(), vertices, colour);
    }

    void DebugShader::draw_line(
        const Camera& camera,
        const geom::Vector3& p1,
        const geom::Vector3& p2,
        float thickness,
        Colour colour)
    {
        //each end is split in two, with each half offset perpendicular to the line and camera offset by thickness/2
        geom::Vector3 offset1 = 0.5f * thickness * geom::Vector3::cross(p1 - camera.translation, p2 - p1).normalized();
        geom::Vector3 offset2 = 0.5f * thickness * geom::Vector3::cross(p2 - camera.translation, p2 - p1).normalized();

        std::vector<VectorVertex> vertices;
        vertices.reserve(6);
        vertices.push_back({ p1 + offset1 });
        vertices.push_back({ p1 - offset1 });
        vertices.push_back({ p2 - offset2 });
        vertices.push_back({ p1 + offset1 });
        vertices.push_back({ p2 - offset2 });
        vertices.push_back({ p2 + offset2 });

        draw(camera.calculate_camera_matrix(), vertices, colour);
    }

    void DebugShader::draw(const geom::Matrix44& camera, const std::vector<VectorVertex>& vertices, Colour colour)
    {
        use();
        set_uniform("camera", camera);
        set_uniform("colour", colour);

        VertexArray vao(VertexBuffer(vertices, GL_STREAM_DRAW), nullptr, 0);
        vao.use();

        glDrawArrays(GL_TRIANGLES, 0, (int)vertices.size());
    }

}
