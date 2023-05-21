#pragma once

#include <fbxsdk.h>

class FBXSceneWrapper
{
    friend class FBXManagerWrapper;
    FBXSceneWrapper() = default;
    void unload();
public:
    ~FBXSceneWrapper();
    FBXSceneWrapper(FBXSceneWrapper&& other) noexcept;
    FBXSceneWrapper& operator=(FBXSceneWrapper&& rhs) noexcept;
    bool is_valid() const { return scene != nullptr; }
    const FbxScene* get_scene() const { return scene; }

private:
    FbxScene* scene = nullptr;
};

class FBXManagerWrapper
{
public:
    FBXManagerWrapper();
    ~FBXManagerWrapper();

    FBXSceneWrapper load(const char* filename);

private:
    FbxManager* m_manager;
};