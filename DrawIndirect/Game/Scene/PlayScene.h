/*
	@file	PlayScene.h
	@brief	�v���C�V�[���N���X
*/
#pragma once
#include "IScene.h"
#include "Game/Mylib/Model3D.h"
// �O���錾
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

	

	// DrawIndexedInstancedIndirect �̈����\����
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
		DirectX::SimpleMath::Vector4 FrustumCorners[8]; // �t���X�^����8���_
		DirectX::SimpleMath::Vector4 FrustumPlanes[6]; // �t���X�^����6���� (xyz:�@��, w:����)
		UINT TotalInstanceCount;
		UINT IndexCountPerInstance;
		UINT padding[2];
	};

	// �C���X�^���X�f�[�^�iGPU�p�j
	struct InstanceData
	{
		DirectX::SimpleMath::Matrix World;
		DirectX::SimpleMath::Vector4 Extents;
	};


private:
	// ���ʃ��\�[�X
	CommonResources* m_commonResources;
	// �f�o�b�O�J����
	std::unique_ptr<DebugCamera> m_debugCamera;
	// �ˉe�s��
	DirectX::SimpleMath::Matrix m_projection;
	
	DirectX::Model* m_model;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;

	std::vector<DirectX::SimpleMath::Matrix> m_worlds;

	ShaderSet m_testSet;
	
	// �Ԑڕ`��p�̈����o�b�t�@
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indirectArgsBuffer;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_indirectArgsUAV;

	// �t���X�^���J�����O�p
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_allInstanceBuffer;        // �S�C���X�^���X
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_allInstanceSRV;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_visibleInstanceBuffer;    // ���C���X�^���X
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
