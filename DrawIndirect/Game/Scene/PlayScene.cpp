/*
	@file	PlayScene.cpp
	@brief	�v���C�V�[���N���X
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



const std::vector<D3D11_INPUT_ELEMENT_DESC> INSTANCE_INPUT_LAYOUT =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "MATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "MATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "MATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "MATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
};

//---------------------------------------------------------
// �R���X�g���N�^
//---------------------------------------------------------
PlayScene::PlayScene()
	:
	m_commonResources{},
	m_debugCamera{},
	m_projection{}
{
}

//---------------------------------------------------------
// �f�X�g���N�^
//---------------------------------------------------------
PlayScene::~PlayScene()
{
	// do nothing.

	
}

//---------------------------------------------------------
// ����������
//---------------------------------------------------------
void PlayScene::Initialize(CommonResources* resources)
{
	assert(resources);
	m_commonResources = resources;

	auto device = m_commonResources->GetDeviceResources()->GetD3DDevice();
	auto context = m_commonResources->GetDeviceResources()->GetD3DDeviceContext();
	auto states = m_commonResources->GetCommonStates();

	// �f�o�b�O�J�������쐬����
	RECT rect{ m_commonResources->GetDeviceResources()->GetOutputSize() };
	m_debugCamera = std::make_unique<DebugCamera>();
	m_debugCamera->Initialize(rect.right, rect.bottom);

	// �ˉe�s����쐬����
	m_projection = SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		XMConvertToRadians(45.0f),
		static_cast<float>(rect.right) / static_cast<float>(rect.bottom),
		0.1f, 1000.0f
	);


	m_model = GameResources::GetInstance()->GetModel("Box");
	m_texture = GameResources::GetInstance()->GetTexture("Box");


	int SIZE = Count;
	// �O���b�h�̑傫�����v�Z�i�����`�ɋ߂��`�ɔz�u�j
	int gridSize = static_cast<int>(ceil(sqrt(static_cast<float>(SIZE))));
	// ���f���Ԃ̋���
	float spacing = 3.0f; // �K�؂ȋ����ɒ������Ă�������
	// �O���b�h�̊J�n�ʒu�i�����ɔz�u���邽�߂ɃI�t�Z�b�g�j
	float startX = -((gridSize - 1) * spacing) / 2.0f;
	float startZ = -((gridSize - 1) * spacing) / 2.0f;

	/*m_models.resize(SIZE);*/
	for (int i = 0; i < SIZE; i++)
	{
		// �O���b�h���̈ʒu���v�Z
		int row = i / gridSize;
		int col = i % gridSize;

		// XZ���ʏ�̈ʒu���v�Z
		float x = startX + col * spacing;
		float z = startZ + row * spacing;

		// Y����0�i���ʏ�j
		float y = 0.0f;

		m_worlds.push_back(Matrix::CreateTranslation(Vector3(x, y, z)));
	}

	m_testSet.vertexShader = ShaderManager::CreateVSShader(device, "DIDVS.cso");
	m_testSet.pixelShader = ShaderManager::CreatePSShader(device, "DIDPS.cso");
	m_testSet.inputLayout = ShaderManager::CreateInputLayout(device, INSTANCE_INPUT_LAYOUT, "DIDVS.cso");
	m_testSet.cBuffer = ShaderManager::CreateConstantBuffer<CBuff>(device);

	// �C���X�^���X�o�b�t�@�̍쐬
	D3D11_BUFFER_DESC instanceBufferDesc = {};
	instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	instanceBufferDesc.ByteWidth = sizeof(WorldBuffer);
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&instanceBufferDesc, nullptr, &m_instancBuffer);


	// �Ԑڕ`��p�̈����o�b�t�@���쐬
	DrawIndexedInstancedIndirectArgs indirectArgs = {};
	indirectArgs.IndexCountPerInstance = 56;
	indirectArgs.InstanceCount = Count;
	indirectArgs.StartIndexLocation = 0;
	indirectArgs.BaseVertexLocation = 0;
	indirectArgs.StartInstanceLocation = 0;

	D3D11_BUFFER_DESC indirectArgsDesc = {};
	indirectArgsDesc.Usage = D3D11_USAGE_DEFAULT;
	indirectArgsDesc.ByteWidth = sizeof(DrawIndexedInstancedIndirectArgs);
	indirectArgsDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	indirectArgsDesc.CPUAccessFlags = 0;
	indirectArgsDesc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
	indirectArgsDesc.StructureByteStride = 0;
	device->CreateBuffer(&indirectArgsDesc, nullptr, &m_indirectArgsBuffer);



	m_computeShader = ShaderManager::CreateCSShader(device, "DIDCS.cso");

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_R32_UINT;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = 5; // DrawIndexedInstancedIndirectArgs�̗v�f��
	uavDesc.Buffer.Flags = 0;
	device->CreateUnorderedAccessView(m_indirectArgsBuffer.Get(), &uavDesc, &m_indirectArgsUAV);

	D3D11_BUFFER_DESC computeCBDesc = {};
	computeCBDesc.Usage = D3D11_USAGE_DYNAMIC;
	computeCBDesc.ByteWidth = sizeof(ComputeConstants);
	computeCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	computeCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	computeCBDesc.MiscFlags = 0;
	device->CreateBuffer(&computeCBDesc, nullptr, &m_computeConstantBuffer);
}

//---------------------------------------------------------
// �X�V����
//---------------------------------------------------------
void PlayScene::Update(float elapsedTime)
{
	UNREFERENCED_PARAMETER(elapsedTime);

	// �f�o�b�O�J�������X�V����
	m_debugCamera->Update(m_commonResources->GetInputManager());
}

//---------------------------------------------------------
// �`�悷��
//---------------------------------------------------------
void PlayScene::Render()
{
	auto device = m_commonResources->GetDeviceResources()->GetD3DDevice();
	auto context = m_commonResources->GetDeviceResources()->GetD3DDeviceContext();
	auto states = m_commonResources->GetCommonStates();
	// �r���[�s����擾����
	const Matrix& view = m_debugCamera->GetViewMatrix();
	Matrix world = DirectX::SimpleMath::Matrix::Identity;

	using namespace DirectX;
	using namespace DirectX::SimpleMath;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	// �萔�o�b�t�@���}�b�v����
	context->Map(m_instancBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	WorldBuffer* cb = static_cast<WorldBuffer*>(mappedResource.pData);
	for (int i = 0; i < Count; i++) {
		cb->mat[i] = m_worlds[i].Transpose();
	}
	context->Unmap(m_instancBuffer.Get(), 0);

	//for (auto& meshes : m_model->meshes)
	//{
	//	for (auto& mesh : meshes->meshParts)
	//	{
	//		ID3D11Buffer* pBuf[2] = { mesh->vertexBuffer.Get(), m_instancBuffer.Get() };

	//		UINT strides[2] = { mesh->vertexStride, sizeof(Matrix) };
	//		UINT offsets[2] = { 0, 0 };

	//		context->IASetVertexBuffers(0, 2, pBuf, strides, offsets);
	//		//�C���f�b�N�X�o�b�t�@�̃Z�b�g
	//		context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	//		//�`����@�i3�p�`�j
	//		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//		//�V�F�[�_�̐ݒ�
	//		context->VSSetShader(m_testSet.vertexShader.Get(), nullptr, 0);
	//		context->PSSetShader(m_testSet.pixelShader.Get(), nullptr, 0);
	//		//�C���v�b�g���C�A�E�g�̐ݒ�
	//		context->IASetInputLayout(m_testSet.inputLayout.Get());

	//		//�u�����h�X�e�[�g
	//		//�����������Ȃ�
	//		context->OMSetBlendState(states->Opaque(), nullptr, 0xffffffff);
	//		//�f�v�X�X�e���V���X�e�[�g
	//		context->OMSetDepthStencilState(states->DepthDefault(), 0);
	//		//�e�N�X�`���ƃT���v���[�̐ݒ�
	//		ID3D11ShaderResourceView* pNull[1] = { 0 };
	//		context->PSSetShaderResources(0, 1, m_texture.GetAddressOf());
	//		ID3D11SamplerState* pSampler = states->LinearWrap();;
	//		context->PSSetSamplers(0, 1, &pSampler);
	//		//���X�^���C�U�X�e�[�g�i�\�ʕ`��j
	//		context->RSSetState(states->CullNone());

	//		//�R���X�^���g�o�b�t�@�̏���
	//		CBuff sb;
	//		sb.World = Matrix::Identity;
	//		sb.View = view.Transpose();
	//		sb.Projection = m_projection.Transpose();
	//		//���C�e�B���O
	//		Vector4 LightDir(0.5f, -1.0f, 0.5f, 0.0f);
	//		LightDir.Normalize();
	//		sb.LightDir = LightDir;
	//		//�f�B�t���[�Y
	//		sb.Diffuse = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//		//�G�~�b�V�u���Z�B
	//		sb.Emissive = Vector4(0.4f, 0.4f, 0.4f, 0);
	//		//�R���X�^���g�o�b�t�@�̍X�V
	//		context->UpdateSubresource(m_testSet.cBuffer.Get(), 0, nullptr, &sb, 0, 0);
	//		//�R���X�^���g�o�b�t�@�̐ݒ�
	//		ID3D11Buffer* pConstantBuffer = m_testSet.cBuffer.Get();
	//		//���_�V�F�[�_�ɓn��
	//		context->VSSetConstantBuffers(0, 1, &pConstantBuffer);
	//		//�s�N�Z���V�F�[�_�ɓn��
	//		context->PSSetConstantBuffers(0, 1, &pConstantBuffer);
	//		//�`��
	//		context->DrawIndexedInstancedIndirect(m_indirectArgsBuffer.Get(), 0);
	//	}
	//}

	for (auto& meshes : m_model->meshes)
	{
		for (auto& mesh : meshes->meshParts)
		{
			// Compute Shader�ŕ`�������ݒ�
			{
				// Compute Shader�p�̒萔�o�b�t�@���X�V
				D3D11_MAPPED_SUBRESOURCE mapped;
				context->Map(m_computeConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
				ComputeConstants* constants = static_cast<ComputeConstants*>(mapped.pData);
				constants->TotalInstanceCount = Count;
				constants->IndexCountPerInstance = mesh->indexCount;
				context->Unmap(m_computeConstantBuffer.Get(), 0);

				// Compute Shader�̐ݒ�
				context->CSSetShader(m_computeShader.Get(), nullptr, 0);
				context->CSSetConstantBuffers(0, 1, m_computeConstantBuffer.GetAddressOf());
				context->CSSetUnorderedAccessViews(0, 1, m_indirectArgsUAV.GetAddressOf(), nullptr);

				// Compute Shader�̎��s�i1�X���b�h�O���[�v�ŏ\���j
				context->Dispatch(1, 1, 1);

				// UAV�̃A���o�C���h
				ID3D11UnorderedAccessView* nullUAV = nullptr;
				context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
			}



			ID3D11Buffer* pBuf[2] = { mesh->vertexBuffer.Get(), m_instancBuffer.Get() };

			UINT strides[2] = { mesh->vertexStride, sizeof(Matrix) };
			UINT offsets[2] = { 0, 0 };

			context->IASetVertexBuffers(0, 2, pBuf, strides, offsets);
			//�C���f�b�N�X�o�b�t�@�̃Z�b�g
			context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
			//�`����@�i3�p�`�j
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			//�V�F�[�_�̐ݒ�
			context->VSSetShader(m_testSet.vertexShader.Get(), nullptr, 0);
			context->PSSetShader(m_testSet.pixelShader.Get(), nullptr, 0);
			//�C���v�b�g���C�A�E�g�̐ݒ�
			context->IASetInputLayout(m_testSet.inputLayout.Get());

			//�u�����h�X�e�[�g
			//�����������Ȃ�
			context->OMSetBlendState(states->Opaque(), nullptr, 0xffffffff);
			//�f�v�X�X�e���V���X�e�[�g
			context->OMSetDepthStencilState(states->DepthDefault(), 0);
			//�e�N�X�`���ƃT���v���[�̐ݒ�
			ID3D11ShaderResourceView* pNull[1] = { 0 };
			context->PSSetShaderResources(0, 1, m_texture.GetAddressOf());
			ID3D11SamplerState* pSampler = states->LinearWrap();;
			context->PSSetSamplers(0, 1, &pSampler);
			//���X�^���C�U�X�e�[�g�i�\�ʕ`��j
			context->RSSetState(states->CullNone());

			//�R���X�^���g�o�b�t�@�̏���
			CBuff sb;
			sb.World = Matrix::Identity;
			sb.View = view.Transpose();
			sb.Projection = m_projection.Transpose();
			//���C�e�B���O
			Vector4 LightDir(0.5f, -1.0f, 0.5f, 0.0f);
			LightDir.Normalize();
			sb.LightDir = LightDir;
			//�f�B�t���[�Y
			sb.Diffuse = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
			//�G�~�b�V�u���Z�B
			sb.Emissive = Vector4(0.4f, 0.4f, 0.4f, 0);
			//�R���X�^���g�o�b�t�@�̍X�V
			context->UpdateSubresource(m_testSet.cBuffer.Get(), 0, nullptr, &sb, 0, 0);
			//�R���X�^���g�o�b�t�@�̐ݒ�
			ID3D11Buffer* pConstantBuffer = m_testSet.cBuffer.Get();
			//���_�V�F�[�_�ɓn��
			context->VSSetConstantBuffers(0, 1, &pConstantBuffer);
			//�s�N�Z���V�F�[�_�ɓn��
			context->PSSetConstantBuffers(0, 1, &pConstantBuffer);
			//�`��
			context->DrawIndexedInstancedIndirect(m_indirectArgsBuffer.Get(), 0);
		}
	}


	// �f�o�b�O�����uDebugString�v�ŕ\������
	auto debugString = m_commonResources->GetDebugString();
}

//---------------------------------------------------------
// ��n������
//---------------------------------------------------------
void PlayScene::Finalize()
{
	// do nothing.
}

//---------------------------------------------------------
// ���̃V�[��ID���擾����
//---------------------------------------------------------
IScene::SceneID PlayScene::GetNextSceneID() const
{
	// �V�[���ύX���Ȃ��ꍇ
	return IScene::SceneID::NONE;
}

