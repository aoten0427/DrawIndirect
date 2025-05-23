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

const int MaxCount = 10000;

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
		0.1f, 100.0f
	);


	m_model = GameResources::GetInstance()->GetModel("Box");

	m_meshBuffer = ShaderManager::CreateConstantBuffer<MeshPartInfo>(device);

	//�o�̓f�[�^
	{
		D3D11_BUFFER_DESC BufferDesc;
		ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
		BufferDesc.BindFlags =
			D3D11_BIND_UNORDERED_ACCESS |
			D3D11_BIND_SHADER_RESOURCE;
		BufferDesc.ByteWidth = sizeof(DrawIndirectArgs);
		BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		BufferDesc.StructureByteStride = sizeof(DrawIndirectArgs);

		device->CreateBuffer(&BufferDesc, nullptr, &m_computeBuffer);

		D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
		ZeroMemory(&UAVDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
		UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		UAVDesc.Buffer.FirstElement = 0;
		UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
		UAVDesc.Buffer.NumElements = 1;

		// �\�����o�b�t�@�[�����ƂɃA���I�[�_�[�h �A�N�Z�X �r���[���쐬����
		device->CreateUnorderedAccessView(m_computeBuffer.Get(), &UAVDesc, &m_outputBufResultUAV);
	}

	{
		// 2. DrawIndirect��p�o�b�t�@
		D3D11_BUFFER_DESC indirectBufferDesc = {};
		indirectBufferDesc.BindFlags = 0;
		indirectBufferDesc.ByteWidth = sizeof(DrawIndirectArgs);
		indirectBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		indirectBufferDesc.Usage = D3D11_USAGE_DEFAULT;

		device->CreateBuffer(&indirectBufferDesc, nullptr, &m_indirectArgsBuffer);
	}

	D3D11_BUFFER_DESC BufferDesc;
	ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
	m_computeBuffer->GetDesc(&BufferDesc);
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	BufferDesc.Usage = D3D11_USAGE_STAGING;
	BufferDesc.BindFlags = 0;
	BufferDesc.MiscFlags = 0;
	device->CreateBuffer(&BufferDesc, nullptr, &m_copyBuffer);



	m_vertexShader = ShaderManager::CreateVSShader(device, "DIDVS.cso");
	m_pixelShader = ShaderManager::CreatePSShader(device, "DIDPS.cso");
	m_computeShader = ShaderManager::CreateCSShader(device, "DIDCS.cso");
	m_inputLayout = ShaderManager::CreateInputLayout(device, MODEL_INPUT_LAYOUT, "DIDVS.cso");
	m_cbuff = ShaderManager::CreateConstantBuffer<CBuff>(device);
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


	


	for (auto& meshes : m_model->meshes)
	{
		for (auto& mesh : meshes->meshParts)
		{

			// �R���s���[�g�V�F�[�_�[�Ńp�����[�^��ݒ�
			MeshPartInfo meshinfo;
			meshinfo.indexCount = mesh->indexCount;
			meshinfo.startIndexLocation = 0;
			meshinfo.baseVertexLocation = 0;
			context->UpdateSubresource(m_meshBuffer.Get(), 0, nullptr, &meshinfo, 0, 0);
			ID3D11Buffer* pconstantBuffer = m_meshBuffer.Get();

			context->CSSetUnorderedAccessViews(0, 1, &m_outputBufResultUAV, nullptr);
			context->CSSetShader(m_computeShader.Get(), nullptr, 0);
			context->CSSetConstantBuffers(0, 1, &pconstantBuffer);
			context->Dispatch(1, 1, 1);

			// UAV�̃A���o�C���h
			ID3D11UnorderedAccessView* nullUAV = nullptr;
			context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
			context->CopyResource(m_indirectArgsBuffer.Get(), m_computeBuffer.Get());






			UINT stride = sizeof(DirectX::VertexPositionNormalTangentColorTexture); // VertexType�͎��ۂ̒��_�\���̂ɍ��킹��
			UINT offset = 0;
			context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(), &stride, &offset);
			////�C���f�b�N�X�o�b�t�@�̃Z�b�g
			context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
			//�`����@�i3�p�`�jD3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			context->IASetInputLayout(m_inputLayout.Get());
			context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
			context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

			CBuff cb;
			DirectX::SimpleMath::Matrix world = Matrix::Identity * view * m_projection;
			cb.mat = world.Transpose();

			//�R���X�^���g�o�b�t�@�̍X�V
			context->UpdateSubresource(m_cbuff.Get(), 0, nullptr, &cb, 0, 0);
			//�R���X�^���g�o�b�t�@�̐ݒ�
			ID3D11Buffer* pConstantBuffer = m_cbuff.Get();
			//���_�V�F�[�_�ɓn��
			context->VSSetConstantBuffers(0, 1, &pConstantBuffer);
			//�s�N�Z���V�F�[�_�ɓn��
			context->PSSetConstantBuffers(0, 1, &pConstantBuffer);

			context->DrawInstancedIndirect(m_indirectArgsBuffer.Get(), 0);
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

