#include "KScene_01.h"
#include "KSceneManager.h"
#include "KState.h"
#include "KFBXManager.h"
#include "ImGuiManager.h"
bool KScene_01::Load(std::wstring file)
{
	//todo:��������� �� ����
	return true;
}

bool KScene_01::Init(ID3D11DeviceContext* context)
{
	KScene::Init(context);
	m_SceneID = S_Scene1;

	//�̴ϸ�-------------------------------------------------------------
	m_MiniMap_DebugShadow.Init(KRect(KVector2(-0.25f, -0.75f),0.5f, 0.5f));
	m_MiniMap_DebugCamera.Init(KRect(KVector2(-0.75f, -0.75f), 0.5f, 0.5f));
	
	//�̴ϸ��� Ű�Է����� ���̰� ���ۺ��� ������ ����
	m_MiniMap_DebugShadow.SwapVisibility();
	m_MiniMap_DebugCamera.SwapVisibility();
	
	//Fbx ���� �ε�-------------------------------------------------------------
	g_FBXManager.LoadTextureData(L"../../data/model/FBXData.txt"); //��ü �ؽ��� ������ ���� ���� �Ľ�
	std::vector<std::wstring> fbx_name_list;
	KDirParser::LoadAllPath(L"../../data/model/Mesh", fbx_name_list); // �ش� ��� FBX ���� �̸� �Ľ�
	m_FBXList.resize(fbx_name_list.size());
	for (int iObj = 0; iObj < m_FBXList.size(); iObj++)
	{
		std::shared_ptr<KFBXAsset> pFbx = std::make_shared<KFBXAsset>();
		pFbx->Init();
		pFbx->m_matWorld._11 = 0.3f;
		pFbx->m_matWorld._22 = 0.3f;
		pFbx->m_matWorld._33 = 0.3f;
		pFbx->SetPosition(KVector3(iObj * 200 - 400, 3, iObj * 200 - 400));
 		pFbx->m_pLoader = g_FBXManager.Load(fbx_name_list[iObj]);
		m_FBXList[iObj]= pFbx;
	}

	//����-------------------------------------------------------------
	m_Terrian.Init(m_pContext);
	m_Terrian.CreateObject(L"../../data/shader/VSPS_Terrain.hlsl",
		L"../../data/shader/VSPS_Terrain.hlsl",
		L"../../data/map/texture/base.jpg",
		L"../../data/map/texture/base_s.jpg",
		L"../../data/map/texture/base_n.jpg");
	
	//����Ʈ �׸���----------------------------------------------------------------
	m_Light.SetLight(KVector3(10.0f,1000.0f,0.0f), KVector3(0.0f, 0.0f, 0.0f));
	m_Shadow.CreateShadow(&m_Light);

	return true;
}

bool KScene_01::Frame()
{
	m_Light.Frame();
	m_Shadow.Frame(); // ������ ��� ���, �������� ��� ,�ؽ��� ��� ���Ѱ�

	#pragma region IMGUI UI <- HERE
	//IMGUI IU 
	ImVec2 ivMin = { static_cast<float>(g_rtClient.right)- static_cast<float>(g_rtClient.right) / 3.5f, 0 };
	ImVec2 ivMax = { static_cast<float>(g_rtClient.right), static_cast<float>(g_rtClient.bottom) };
	
	ImGui::SetNextWindowSize(ImVec2(ivMax.x -ivMin.x, ivMax.y - ivMin.y));
	ImGui::SetNextWindowPos(ivMin);
	if (ImGui::Begin("Inspector"))
	{
		//----------------------------------------------------------------------------------------
		float* lightColor[3] = { &m_Light.m_vLightColor.x,&m_Light.m_vLightColor.y,&m_Light.m_vLightColor.z };
		if (ImGui::CollapsingHeader("[ Light ]"))
		{
			if (ImGui::BeginChild("light", ImVec2(0, 150), true))
			{
				ImGui::SliderFloat("X", &m_Light.m_vPos.x, -200.0f, 200.0f);
				ImGui::SliderFloat("Z", &m_Light.m_vPos.z, -200.0f, 200.0f);
				ImGui::InputFloat3("Color", *lightColor, 2, 0);
			}
			ImGui::EndChild();
		}
		//----------------------------------------------------------------------------------------	
	}
	ImGui::End();
#pragma endregion

	KScene::Frame();
	return true;
}

bool KScene_01::Render()
{
	//�׸��� ------------------------------------------
	float shadow[4] = { 1.0f,1.0f,1.0f,1.0f };
	if (m_Shadow.m_ShadowRT.Begin(m_pContext, shadow))
	{
		//����Ʈ ���⿡�� ĸ��
		m_Terrian.SetMatrix(&m_Terrian.m_matWorld,&m_Light.m_matView, &m_Light.m_matProj);
		//���̴� ���̴��� ��ü
		m_Terrian.SwapPSShader(m_Shadow.m_pPSShadow);
		m_Terrian.Render(m_pContext);
		//FBX OBJ Render------------------------------------------
		for (int iObj = 0; iObj < m_FBXList.size(); iObj++)
		{
			m_FBXList[iObj]->SetMatrix(&m_FBXList[iObj]->m_matWorld, &m_Light.m_matView, &m_Light.m_matProj);
			m_FBXList[iObj]->SwapPSShader(m_Shadow.m_pPSShadow);
			m_FBXList[iObj]->Render(m_pContext);
		}
		//���� �۾�
		m_Shadow.m_ShadowRT.End(m_pContext);
	}
	m_Shadow.m_Shadow_cbData.m_matShadow = m_Shadow.m_Shadow_cbData.m_matShadow.Transpose();
	m_pContext->UpdateSubresource(
		m_Shadow.m_pShadowCB.Get(), 0, NULL, &m_Shadow.m_Shadow_cbData, 0, 0);
	//������� 2��° ���Կ� ����
	m_pContext->VSSetConstantBuffers(2, 1, m_Shadow.m_pShadowCB.GetAddressOf());
	//���÷� ���� : Ŭ���� -> �׸��ڿ�
	ApplySS(m_pContext, KState::g_pClampSS, 1);
	D3DKMatrixInverse(&m_Terrian.m_cbData.matNormal, NULL,
		&m_Terrian.m_matWorld);
	
	//���� ����------------------------------------------------------------
	m_Terrian.SetMatrix(&m_Terrian.m_matWorld, &g_SceneManager.m_pCamera->m_matView, &g_SceneManager.m_pCamera->m_matProj);
	m_Terrian.m_cbDataEX.vLightColor = { m_Light.m_vLightColor.x,m_Light.m_vLightColor.y,m_Light.m_vLightColor.z,1.0f };
	m_Terrian.m_cbDataEX.vLightPos = { m_Light.m_vPos.x,m_Light.m_vPos.y,m_Light.m_vPos.z,1.0f};
	m_Terrian.m_cbDataEX.vLightDir =   { m_Light.m_vDir.x,m_Light.m_vDir.y,m_Light.m_vDir.z,1.0f };
	m_Terrian.m_cbDataEX.vCamPos = { g_SceneManager.m_pCamera->GetCameraPos()->x, g_SceneManager.m_pCamera->GetCameraPos()->y, g_SceneManager.m_pCamera->GetCameraPos()->z, 1.0f };
	m_pContext->PSSetShaderResources(3, 1, m_Shadow.m_ShadowRT.m_pTextureSRV.GetAddressOf());
	m_Terrian.Render(m_pContext);
	//FBX OBJ Render------------------------------------------
	for (int iObj = 0; iObj < m_FBXList.size(); iObj++)
	{
		m_FBXList[iObj]->SetMatrix(&m_FBXList[iObj]->m_matWorld, &g_SceneManager.m_pCamera->m_matView, &g_SceneManager.m_pCamera->m_matProj);
		m_FBXList[iObj]-> m_cbDataEX.vLightColor = { m_Light.m_vLightColor.x,m_Light.m_vLightColor.y,m_Light.m_vLightColor.z,1.0f };
		m_FBXList[iObj]-> m_cbDataEX.vLightPos = { m_Light.m_vPos.x,m_Light.m_vPos.y,m_Light.m_vPos.z,1.0f };
		m_FBXList[iObj]-> m_cbDataEX.vLightDir = { m_Light.m_vDir.x,m_Light.m_vDir.y,m_Light.m_vDir.z,1.0f };
		m_FBXList[iObj]-> m_cbDataEX.vCamPos = { g_SceneManager.m_pCamera->GetCameraPos()->x, g_SceneManager.m_pCamera->GetCameraPos()->y, g_SceneManager.m_pCamera->GetCameraPos()->z, 1.0f };
		m_pContext->PSSetShaderResources(3, 1, m_Shadow.m_ShadowRT.m_pTextureSRV.GetAddressOf());
		m_FBXList[iObj]->SwapPSShader();
		m_FBXList[iObj]->Render(m_pContext);
	}

	////�̴ϸ�------------------------------------------------
	if (g_InputData.bDebugRender)
	{
		m_MiniMap_DebugShadow.SwapVisibility();
		m_MiniMap_DebugCamera.SwapVisibility();
	}
	m_MiniMap_DebugShadow.SetMatrix(nullptr, nullptr, nullptr);
	m_MiniMap_DebugShadow.PreRender(m_pContext);
	m_pContext->PSSetShaderResources(0, 1, m_Shadow.m_ShadowRT.m_pTextureSRV.GetAddressOf());
	m_MiniMap_DebugShadow.PostRender(m_pContext, m_MiniMap_DebugShadow.m_iNumIndex);

	KScene::Render();
	return true;
}

bool KScene_01::Release()
{
	m_FbxLoader.Release();
	m_Terrian.Release();
	m_MiniMap_DebugCamera.Release();
	m_MiniMap_DebugShadow.Release();
	KScene::Release();
	return true;
}
