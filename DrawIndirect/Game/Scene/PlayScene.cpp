/*
	@file	PlayScene.cpp
	@brief	プレイシーンクラス
*/
#include "pch.h"
#include "PlayScene.h"
#include "Game/CommonResources.h"
#include "DeviceResources.h"
#include "Game/MyLib/DebugCamera.h"
#include "Game/MyLib/DebugString.h"
#include "Game/MyLib/InputManager.h"
#include "Game/Mylib/GameResources.h"
#include <cassert>

using namespace DirectX;
using namespace DirectX::SimpleMath;

const int MaxCount = 10000;

//---------------------------------------------------------
// コンストラクタ
//---------------------------------------------------------
PlayScene::PlayScene()
	:
	m_commonResources{},
	m_debugCamera{},
	m_projection{}
{
}

//---------------------------------------------------------
// デストラクタ
//---------------------------------------------------------
PlayScene::~PlayScene()
{
	// do nothing.

	
}

//---------------------------------------------------------
// 初期化する
//---------------------------------------------------------
void PlayScene::Initialize(CommonResources* resources)
{
	assert(resources);
	m_commonResources = resources;

	auto device = m_commonResources->GetDeviceResources()->GetD3DDevice();
	auto context = m_commonResources->GetDeviceResources()->GetD3DDeviceContext();
	auto states = m_commonResources->GetCommonStates();

	// デバッグカメラを作成する
	RECT rect{ m_commonResources->GetDeviceResources()->GetOutputSize() };
	m_debugCamera = std::make_unique<DebugCamera>();
	m_debugCamera->Initialize(rect.right, rect.bottom);

	// 射影行列を作成する
	m_projection = SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		XMConvertToRadians(45.0f),
		static_cast<float>(rect.right) / static_cast<float>(rect.bottom),
		0.1f, 100.0f
	);


	m_model = GameResources::GetInstance()->GetModel("Box");
}

//---------------------------------------------------------
// 更新する
//---------------------------------------------------------
void PlayScene::Update(float elapsedTime)
{
	UNREFERENCED_PARAMETER(elapsedTime);

	// デバッグカメラを更新する
	m_debugCamera->Update(m_commonResources->GetInputManager());
}

//---------------------------------------------------------
// 描画する
//---------------------------------------------------------
void PlayScene::Render()
{
	auto context = m_commonResources->GetDeviceResources()->GetD3DDeviceContext();
	auto states = m_commonResources->GetCommonStates();
	// ビュー行列を取得する
	const Matrix& view = m_debugCamera->GetViewMatrix();
	Matrix world = DirectX::SimpleMath::Matrix::Identity;

	using namespace DirectX;
	using namespace DirectX::SimpleMath;

	m_model->Draw(context, *states, world, view, m_projection);

	// デバッグ情報を「DebugString」で表示する
	auto debugString = m_commonResources->GetDebugString();
}

//---------------------------------------------------------
// 後始末する
//---------------------------------------------------------
void PlayScene::Finalize()
{
	// do nothing.
}

//---------------------------------------------------------
// 次のシーンIDを取得する
//---------------------------------------------------------
IScene::SceneID PlayScene::GetNextSceneID() const
{
	// シーン変更がない場合
	return IScene::SceneID::NONE;
}

