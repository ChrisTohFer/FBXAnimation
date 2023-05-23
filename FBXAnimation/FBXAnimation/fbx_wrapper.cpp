#include "fbx_wrapper.h"

#include <iostream>

//Scene wrapper

void FBXSceneWrapper::unload()
{
    if (scene)
        scene->Destroy();
}

FBXSceneWrapper::~FBXSceneWrapper()
{
    unload();
}

FBXSceneWrapper::FBXSceneWrapper(FBXSceneWrapper&& other) noexcept
    : scene(other.scene)
{
    other.scene = nullptr;
}

FBXSceneWrapper& FBXSceneWrapper::operator=(FBXSceneWrapper&& rhs) noexcept
{
    unload();
    scene = rhs.scene;
    rhs.scene = nullptr;

    return *this;
}

//Manager wrapper

FBXManagerWrapper::FBXManagerWrapper()
{
    m_manager = FbxManager::Create();
    FbxIOSettings* ioSettings = FbxIOSettings::Create(m_manager, "");
    m_manager->SetIOSettings(ioSettings);
}

FBXManagerWrapper::~FBXManagerWrapper()
{
    if (m_manager)
    {
        m_manager->Destroy();
        m_manager = nullptr;
    }
}

FBXSceneWrapper FBXManagerWrapper::load(const char* filename)
{
    FBXSceneWrapper result;
    FbxImporter* importer = FbxImporter::Create(m_manager, "");

    if (!importer->Initialize(filename, -1, m_manager->GetIOSettings()))
    {
        //error
        printf("Call to FbxImporter::Initialize() failed.\n");
        printf("Error returned: %s\n\n", importer->GetStatus().GetErrorString());
        return result;
    }

    FbxScene* scene = FbxScene::Create(m_manager, filename);
    importer->Import(scene);
    importer->Destroy();

    //only want to draw triangles and not quads, so convert the scene immediately
    FbxGeometryConverter converter(m_manager);
    converter.Triangulate(scene, true);

    result.scene = scene;
    return result;
}