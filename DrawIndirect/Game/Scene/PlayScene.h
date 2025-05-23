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


class PlayScene final :
    public IScene
{
private:
	struct MeshPartInfo {
		UINT indexCount;
		UINT startIndexLocation;
		UINT baseVertexLocation;
		UINT padding;  // 16�o�C�g�A���C�����g�p
	};

	struct DrawIndirectArgs {
		UINT VertexCountPerInstance;
		UINT InstanceCount;
		UINT StartVertexLocation;
		UINT StartInstanceLocation;
	};

	struct CBuff
	{
		DirectX::SimpleMath::Matrix mat;
	};
private:
	// ���ʃ��\�[�X
	CommonResources* m_commonResources;
	// �f�o�b�O�J����
	std::unique_ptr<DebugCamera> m_debugCamera;
	// �ˉe�s��
	DirectX::SimpleMath::Matrix m_projection;
	
	DirectX::Model* m_model;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_cbuff;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_meshBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_computeBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indirectArgsBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_copyBuffer;


	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	ID3D11ShaderResourceView* m_inputBufSRV;
	ID3D11UnorderedAccessView* m_outputBufResultUAV;

	DrawIndirectArgs data;
public:
	PlayScene();
	~PlayScene() override;

	void Initialize(CommonResources* resources) override;
	void Update(float elapsedTime)override;
	void Render() override;
	void Finalize() override;

	SceneID GetNextSceneID() const;
};
