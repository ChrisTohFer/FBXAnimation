#pragma once

#include "camera.h"
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

}
