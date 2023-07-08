#pragma once

#include "animation/anim_pose.h"

#include <fbxsdk.h>
#include <glad/glad.h>

#include <memory>

struct SkinnedVertex
{
    geom::Vector3 pos;
    int skinned_bone_index;

    static void apply_attributes()
    {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribIPointer(1, 1, GL_INT, sizeof(SkinnedVertex), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
};

struct FbxFileContent
{
    struct NamedAnim
    {
        std::string name;
        anim::Animation animation;
    };
    std::vector<SkinnedVertex> vertices;
    std::vector<unsigned int> indices;

    std::unique_ptr<anim::Skeleton> skeleton;
    std::vector<NamedAnim> animations;
};

class FBXManagerWrapper
{
public:
    FBXManagerWrapper();
    ~FBXManagerWrapper();

    FbxFileContent load_file_content(const char* filename);

private:
    FbxManager* m_manager;
};