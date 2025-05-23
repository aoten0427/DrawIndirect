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
		0.1f, 1000.0f
	);


	m_model = GameResources::GetInstance()->GetModel("Box");
	m_texture = GameResources::GetInstance()->GetTexture("Box");


	int SIZE = Count;
	// グリッドの大きさを計算（正方形に近い形に配置）
	int gridSize = static_cast<int>(ceil(sqrt(static_cast<float>(SIZE))));
	// モデル間の距離
	float spacing = 3.0f; // 適切な距離に調整してください
	// グリッドの開始位置（中央に配置するためにオフセット）
	float startX = -((gridSize - 1) * spacing) / 2.0f;
	float startZ = -((gridSize - 1) * spacing) / 2.0f;

	/*m_models.resize(SIZE);*/
	for (int i = 0; i < SIZE; i++)
	{
		// グリッド内の位置を計算
		int row = i / gridSize;
		int col = i % gridSize;

		// XZ平面上の位置を計算
		float x = startX + col * spacing;
		float z = startZ + row * spacing;

		// Y軸は0（平面上）
		float y = 0.0f;

		m_worlds.push_back(Matrix::CreateTranslation(Vector3(x, y, z)));
	}

	m_testSet.vertexShader = ShaderManager::CreateVSShader(device, "DIDVS.cso");
	m_testSet.pixelShader = ShaderManager::CreatePSShader(device, "DIDPS.cso");
	m_testSet.inputLayout = ShaderManager::CreateInputLayout(device, INSTANCE_INPUT_LAYOUT, "DIDVS.cso");
	m_testSet.cBuffer = ShaderManager::CreateConstantBuffer<CBuff>(device);

	// インスタンスバッファの作成
	D3D11_BUFFER_DESC instanceBufferDesc = {};
	instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	instanceBufferDesc.ByteWidth = sizeof(WorldBuffer);
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&instanceBufferDesc, nullptr, &m_instancBuffer);


	// 間接描画用の引数バッファを作成
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
	uavDesc.Buffer.NumElements = 5; // DrawIndexedInstancedIndirectArgsの要素数
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
	auto device = m_commonResources->GetDeviceResources()->GetD3DDevice();
	auto context = m_commonResources->GetDeviceResources()->GetD3DDeviceContext();
	auto states = m_commonResources->GetCommonStates();
	// ビュー行列を取得する
	const Matrix& view = m_debugCamera->GetViewMatrix();
	Matrix world = DirectX::SimpleMath::Matrix::Identity;

	using namespace DirectX;
	using namespace DirectX::SimpleMath;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	// 定数バッファをマップする
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
	//		//インデックスバッファのセット
	//		context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	//		//描画方法（3角形）
	//		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//		//シェーダの設定
	//		context->VSSetShader(m_testSet.vertexShader.Get(), nullptr, 0);
	//		context->PSSetShader(m_testSet.pixelShader.Get(), nullptr, 0);
	//		//インプットレイアウトの設定
	//		context->IASetInputLayout(m_testSet.inputLayout.Get());

	//		//ブレンドステート
	//		//透明処理しない
	//		context->OMSetBlendState(states->Opaque(), nullptr, 0xffffffff);
	//		//デプスステンシルステート
	//		context->OMSetDepthStencilState(states->DepthDefault(), 0);
	//		//テクスチャとサンプラーの設定
	//		ID3D11ShaderResourceView* pNull[1] = { 0 };
	//		context->PSSetShaderResources(0, 1, m_texture.GetAddressOf());
	//		ID3D11SamplerState* pSampler = states->LinearWrap();;
	//		context->PSSetSamplers(0, 1, &pSampler);
	//		//ラスタライザステート（表面描画）
	//		context->RSSetState(states->CullNone());

	//		//コンスタントバッファの準備
	//		CBuff sb;
	//		sb.World = Matrix::Identity;
	//		sb.View = view.Transpose();
	//		sb.Projection = m_projection.Transpose();
	//		//ライティング
	//		Vector4 LightDir(0.5f, -1.0f, 0.5f, 0.0f);
	//		LightDir.Normalize();
	//		sb.LightDir = LightDir;
	//		//ディフューズ
	//		sb.Diffuse = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//		//エミッシブ加算。
	//		sb.Emissive = Vector4(0.4f, 0.4f, 0.4f, 0);
	//		//コンスタントバッファの更新
	//		context->UpdateSubresource(m_testSet.cBuffer.Get(), 0, nullptr, &sb, 0, 0);
	//		//コンスタントバッファの設定
	//		ID3D11Buffer* pConstantBuffer = m_testSet.cBuffer.Get();
	//		//頂点シェーダに渡す
	//		context->VSSetConstantBuffers(0, 1, &pConstantBuffer);
	//		//ピクセルシェーダに渡す
	//		context->PSSetConstantBuffers(0, 1, &pConstantBuffer);
	//		//描画
	//		context->DrawIndexedInstancedIndirect(m_indirectArgsBuffer.Get(), 0);
	//	}
	//}

	for (auto& meshes : m_model->meshes)
	{
		for (auto& mesh : meshes->meshParts)
		{
			// Compute Shaderで描画引数を設定
			{
				// Compute Shader用の定数バッファを更新
				D3D11_MAPPED_SUBRESOURCE mapped;
				context->Map(m_computeConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
				ComputeConstants* constants = static_cast<ComputeConstants*>(mapped.pData);
				constants->TotalInstanceCount = Count;
				constants->IndexCountPerInstance = mesh->indexCount;
				context->Unmap(m_computeConstantBuffer.Get(), 0);

				// Compute Shaderの設定
				context->CSSetShader(m_computeShader.Get(), nullptr, 0);
				context->CSSetConstantBuffers(0, 1, m_computeConstantBuffer.GetAddressOf());
				context->CSSetUnorderedAccessViews(0, 1, m_indirectArgsUAV.GetAddressOf(), nullptr);

				// Compute Shaderの実行（1スレッドグループで十分）
				context->Dispatch(1, 1, 1);

				// UAVのアンバインド
				ID3D11UnorderedAccessView* nullUAV = nullptr;
				context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
			}



			ID3D11Buffer* pBuf[2] = { mesh->vertexBuffer.Get(), m_instancBuffer.Get() };

			UINT strides[2] = { mesh->vertexStride, sizeof(Matrix) };
			UINT offsets[2] = { 0, 0 };

			context->IASetVertexBuffers(0, 2, pBuf, strides, offsets);
			//インデックスバッファのセット
			context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
			//描画方法（3角形）
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			//シェーダの設定
			context->VSSetShader(m_testSet.vertexShader.Get(), nullptr, 0);
			context->PSSetShader(m_testSet.pixelShader.Get(), nullptr, 0);
			//インプットレイアウトの設定
			context->IASetInputLayout(m_testSet.inputLayout.Get());

			//ブレンドステート
			//透明処理しない
			context->OMSetBlendState(states->Opaque(), nullptr, 0xffffffff);
			//デプスステンシルステート
			context->OMSetDepthStencilState(states->DepthDefault(), 0);
			//テクスチャとサンプラーの設定
			ID3D11ShaderResourceView* pNull[1] = { 0 };
			context->PSSetShaderResources(0, 1, m_texture.GetAddressOf());
			ID3D11SamplerState* pSampler = states->LinearWrap();;
			context->PSSetSamplers(0, 1, &pSampler);
			//ラスタライザステート（表面描画）
			context->RSSetState(states->CullNone());

			//コンスタントバッファの準備
			CBuff sb;
			sb.World = Matrix::Identity;
			sb.View = view.Transpose();
			sb.Projection = m_projection.Transpose();
			//ライティング
			Vector4 LightDir(0.5f, -1.0f, 0.5f, 0.0f);
			LightDir.Normalize();
			sb.LightDir = LightDir;
			//ディフューズ
			sb.Diffuse = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
			//エミッシブ加算。
			sb.Emissive = Vector4(0.4f, 0.4f, 0.4f, 0);
			//コンスタントバッファの更新
			context->UpdateSubresource(m_testSet.cBuffer.Get(), 0, nullptr, &sb, 0, 0);
			//コンスタントバッファの設定
			ID3D11Buffer* pConstantBuffer = m_testSet.cBuffer.Get();
			//頂点シェーダに渡す
			context->VSSetConstantBuffers(0, 1, &pConstantBuffer);
			//ピクセルシェーダに渡す
			context->PSSetConstantBuffers(0, 1, &pConstantBuffer);
			//描画
			context->DrawIndexedInstancedIndirect(m_indirectArgsBuffer.Get(), 0);
		}
	}


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

