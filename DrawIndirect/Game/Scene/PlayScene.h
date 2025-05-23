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

	// Compute Shader�p�̒萔�o�b�t�@
	struct ComputeConstants
	{
		UINT TotalInstanceCount;
		UINT IndexCountPerInstance;
		UINT padding[2]; // 16�o�C�g�A���C�����g
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
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_instancBuffer;

	// �Ԑڕ`��p�̈����o�b�t�@
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
