#pragma once
#include "KScene.h"
#include "KCamera.h"
#include "KSoundManager.h"
#include "KMap.h"
#include "KMiniMap.h"
#include "KRenderTarget.h"
#include "KDepthShadow.h"
#include "KFBXAsset.h"
class KScene_01 : public KScene
{
public:
    KLight         m_Light;
    KDepthShadow   m_Shadow;
public:
    KFbxLoader	   m_FbxLoader;
    std::vector<std::shared_ptr<KFBXAsset>> m_FBXList;
    std::vector<KTexture*>  m_TextureList;
public:
    KMap           m_Terrian;
public:
    KMiniMap       m_MiniMap_DebugCamera;
    KMiniMap       m_MiniMap_DebugShadow;
public:
    bool    Load(std::wstring file) override;
public:
    virtual bool	Init(ID3D11DeviceContext* context);
    virtual bool	Frame();
    virtual bool	Render();
    virtual bool	Release();
};

