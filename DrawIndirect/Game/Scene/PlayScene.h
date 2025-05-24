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

	struct ComputeConstants
	{
		DirectX::SimpleMath::Vector4 FrustumCorners[8]; // フラスタムの8頂点
		DirectX::SimpleMath::Vector4 FrustumPlanes[6]; // フラスタムの6平面 (xyz:法線, w:距離)
		UINT TotalInstanceCount;
		UINT IndexCountPerInstance;
		UINT padding[2];
	};

	// インスタンスデータ（GPU用）
	struct InstanceData
	{
		DirectX::SimpleMath::Matrix World;
		DirectX::SimpleMath::Vector4 Extents;
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
	
	// 間接描画用の引数バッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indirectArgsBuffer;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_indirectArgsUAV;

	// フラスタムカリング用
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_allInstanceBuffer;        // 全インスタンス
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_allInstanceSRV;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_visibleInstanceBuffer;    // 可視インスタンス
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_visibleInstanceUAV;

	Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_computeConstantBuffer;
	
	DirectX::BoundingFrustum m_frustum;


	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_primitiveBatch;
	std::unique_ptr<DirectX::BasicEffect> m_basicEffect;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	float m_rotate = 0;
public:
	PlayScene();
	~PlayScene() override;

	void Initialize(CommonResources* resources) override;
	void Update(float elapsedTime)override;
	void Render() override;
	void Finalize() override;

	SceneID GetNextSceneID() const;

	void MakeFrustumData(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix projection,ComputeConstants& data);
	DirectX::SimpleMath::Vector4 CalculatePlane(const DirectX::SimpleMath::Vector3& a, const DirectX::SimpleMath::Vector3& b, const DirectX::SimpleMath::Vector3& c);
};
