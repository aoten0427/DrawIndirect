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

const int Count = 1000000;

class PlayScene final :
    public IScene
{
private:
	struct CBuff
	{
		DirectX::SimpleMath::Matrix World;
		DirectX::SimpleMath::Matrix View;
		DirectX::SimpleMath::Matrix Projection;
		DirectX::SimpleMath::Vector4 LightDir;
		DirectX::SimpleMath::Vector4 Emissive;
		DirectX::SimpleMath::Vector4 Diffuse;
	};

	struct WorldBuffer
	{
		DirectX::SimpleMath::Matrix mat[Count];
	};

	

	// DrawIndexedInstancedIndirect の引数構造体
	struct DrawIndexedInstancedIndirectArgs
	{
		UINT IndexCountPerInstance;
		UINT InstanceCount;
		UINT StartIndexLocation;
		INT BaseVertexLocation;
		UINT StartInstanceLocation;
	};

	// Compute Shader用の定数バッファ
	struct ComputeConstants
	{
		UINT TotalInstanceCount;
		UINT IndexCountPerInstance;
		UINT padding[2]; // 16バイトアライメント
	};
private:
	// 共通リソース
	CommonResources* m_commonResources;
	// デバッグカメラ
	std::unique_ptr<DebugCamera> m_debugCamera;
	// 射影行列
	DirectX::SimpleMath::Matrix m_projection;
	
	DirectX::Model* m_model;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;

	std::vector<DirectX::SimpleMath::Matrix> m_worlds;

	ShaderSet m_testSet;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_instancBuffer;

	// 間接描画用の引数バッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indirectArgsBuffer;


	Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_computeConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_indirectArgsUAV;
public:
	PlayScene();
	~PlayScene() override;

	void Initialize(CommonResources* resources) override;
	void Update(float elapsedTime)override;
	void Render() override;
	void Finalize() override;

	SceneID GetNextSceneID() const;
};
