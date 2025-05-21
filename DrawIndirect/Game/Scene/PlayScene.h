/*
	@file	PlayScene.h
	@brief	プレイシーンクラス
*/
#pragma once
#include "IScene.h"
#include "Game/Mylib/Model3D.h"
// 前方宣言
class CommonResources;
class DebugCamera;


class PlayScene final :
    public IScene
{
private:
	
	// ドローコール情報を格納するための構造体
	struct DrawCallArguments
	{
		UINT IndexCountPerInstance;
		UINT InstanceCount;
		UINT StartIndexLocation;
		INT BaseVertexLocation;
		UINT StartInstanceLocation;
	};
private:
	// 共通リソース
	CommonResources* m_commonResources;
	// デバッグカメラ
	std::unique_ptr<DebugCamera> m_debugCamera;
	// 射影行列
	DirectX::SimpleMath::Matrix m_projection;
	
	DirectX::Model* m_model;
	
public:
	PlayScene();
	~PlayScene() override;

	void Initialize(CommonResources* resources) override;
	void Update(float elapsedTime)override;
	void Render() override;
	void Finalize() override;

	SceneID GetNextSceneID() const;
};
