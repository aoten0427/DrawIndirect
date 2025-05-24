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
#include "Libraries/Microsoft/DebugDraw.h"

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
	m_projection{},
	m_rotate(0.0f)
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

	//プリミティブバッチ作成
	m_primitiveBatch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

	// ベーシックエフェクトを作成する
	m_basicEffect = std::make_unique<BasicEffect>(device);
	m_basicEffect->SetVertexColorEnabled(true);
	m_basicEffect->SetLightingEnabled(false);
	m_basicEffect->SetTextureEnabled(false);

	// 入力レイアウトを作成する
	DX::ThrowIfFailed(
		CreateInputLayoutFromEffect<VertexPositionColor>(
			device,
			m_basicEffect.get(),
			m_inputLayout.ReleaseAndGetAddressOf()
		)
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

	// 全インスタンスバッファの作成（Structured Buffer）
	{
		D3D11_BUFFER_DESC allInstanceDesc = {};
		allInstanceDesc.Usage = D3D11_USAGE_DEFAULT;
		allInstanceDesc.ByteWidth = sizeof(InstanceData) * Count;
		allInstanceDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		allInstanceDesc.CPUAccessFlags = 0;
		allInstanceDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		allInstanceDesc.StructureByteStride = sizeof(InstanceData);

		std::vector<InstanceData> instanceData(Count);
		for (int i = 0; i < Count; i++)
		{
			instanceData[i].World = m_worlds[i];
			instanceData[i].Extents = Vector4(1, 1, 1, 0);
		}

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = instanceData.data();

		HRESULT hr = device->CreateBuffer(&allInstanceDesc, &initData, &m_allInstanceBuffer);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create all instance buffer");
		}

		// SRVの作成
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = Count;

		hr = device->CreateShaderResourceView(m_allInstanceBuffer.Get(), &srvDesc, &m_allInstanceSRV);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create SRV for all instances");
		}
	}

	// 可視インスタンスバッファの作成（頂点バッファとして使用）
	{
		D3D11_BUFFER_DESC visibleInstanceDesc = {};
		visibleInstanceDesc.Usage = D3D11_USAGE_DEFAULT;
		visibleInstanceDesc.ByteWidth = sizeof(Matrix) * Count;
		visibleInstanceDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_UNORDERED_ACCESS;
		visibleInstanceDesc.CPUAccessFlags = 0;
		visibleInstanceDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		visibleInstanceDesc.StructureByteStride = 0;

		HRESULT hr = device->CreateBuffer(&visibleInstanceDesc, nullptr, &m_visibleInstanceBuffer);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create visible instance buffer");
		}

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = Count * 16; // Matrix = 16 floats
		uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;

		hr = device->CreateUnorderedAccessView(m_visibleInstanceBuffer.Get(), &uavDesc, &m_visibleInstanceUAV);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create UAV for visible instances");
		}
	}

	// 間接描画用の引数バッファを作成
	{
		DrawIndexedInstancedIndirectArgs indirectArgs = {};
		indirectArgs.IndexCountPerInstance = 0; // Compute Shaderで設定
		indirectArgs.InstanceCount = 0;         // Compute Shaderで設定
		indirectArgs.StartIndexLocation = 0;
		indirectArgs.BaseVertexLocation = 0;
		indirectArgs.StartInstanceLocation = 0;

		D3D11_BUFFER_DESC indirectArgsDesc = {};
		indirectArgsDesc.Usage = D3D11_USAGE_DEFAULT;
		indirectArgsDesc.ByteWidth = sizeof(DrawIndexedInstancedIndirectArgs);
		indirectArgsDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		indirectArgsDesc.CPUAccessFlags = 0;
		indirectArgsDesc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		indirectArgsDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = &indirectArgs;

		HRESULT hr = device->CreateBuffer(&indirectArgsDesc, &initData, &m_indirectArgsBuffer);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create indirect args buffer");
		}

		// UAVの作成（間接描画引数用）
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_R32_UINT;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = 5; // DrawIndexedInstancedIndirectArgsの要素数
		uavDesc.Buffer.Flags = 0;

		hr = device->CreateUnorderedAccessView(m_indirectArgsBuffer.Get(), &uavDesc, &m_indirectArgsUAV);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create UAV for indirect args buffer");
		}
	}

	// Compute Shaderの作成
	m_computeShader = ShaderManager::CreateCSShader(device, "DIDCS.cso");

	// Compute Shader用の定数バッファ
	{
		D3D11_BUFFER_DESC computeCBDesc = {};
		computeCBDesc.Usage = D3D11_USAGE_DYNAMIC;
		computeCBDesc.ByteWidth = sizeof(ComputeConstants);
		computeCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		computeCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		computeCBDesc.MiscFlags = 0;

		HRESULT hr = device->CreateBuffer(&computeCBDesc, nullptr, &m_computeConstantBuffer);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create compute constant buffer");
		}
	}
}

//---------------------------------------------------------
// 更新する
//---------------------------------------------------------
void PlayScene::Update(float elapsedTime)
{
	UNREFERENCED_PARAMETER(elapsedTime);

	// デバッグカメラを更新する
	m_debugCamera->Update(m_commonResources->GetInputManager());

	auto input = m_commonResources->GetInputManager();
	if (input->GetKeyboardState().A)
	{
		m_rotate++;
	}
	if (input->GetKeyboardState().D)
	{
		m_rotate--;
	}
}

//---------------------------------------------------------
// 描画する
//---------------------------------------------------------
void PlayScene::Render()
{
	auto device = m_commonResources->GetDeviceResources()->GetD3DDevice();
	auto context = m_commonResources->GetDeviceResources()->GetD3DDeviceContext();
	auto states = m_commonResources->GetCommonStates();

	// フレームの開始時に前フレームのバインディングをクリア
	ID3D11Buffer* nullBuffers[2] = { nullptr, nullptr };
	UINT nullStrides[2] = { 0, 0 };
	UINT nullOffsets[2] = { 0, 0 };
	context->IASetVertexBuffers(0, 2, nullBuffers, nullStrides, nullOffsets);

	// ビュー行列を取得する
	const Matrix& view = m_debugCamera->GetViewMatrix();
	Matrix world = DirectX::SimpleMath::Matrix::Identity;

	auto rotate = Quaternion::CreateFromAxisAngle(Vector3::UnitY, XMConvertToRadians(m_rotate));

	context->OMSetBlendState(states->Opaque(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(states->DepthDefault(), 0);
	context->RSSetState(states->CullClockwise());


	m_basicEffect->SetWorld(world);
	m_basicEffect->SetView(view);
	m_basicEffect->SetProjection(m_projection);
	m_basicEffect->Apply(context);

	context->IASetInputLayout(m_inputLayout.Get());

	ComputeConstants data;
	Vector3 eye = Vector3(0, 0, 0);
	Vector3 target = Vector3::Transform(Vector3(1, 0, 0), rotate);
	Vector3 up = Vector3::UnitY;
	Matrix testview = Matrix::CreateLookAt(eye, target, up);
	MakeFrustumData(testview, m_projection, data);
	m_primitiveBatch->Begin();
	DX::Draw(m_primitiveBatch.get(), m_frustum);
	m_primitiveBatch->End();

	using namespace DirectX;
	using namespace DirectX::SimpleMath;

	for (auto& meshes : m_model->meshes)
	{
		for (auto& mesh : meshes->meshParts)
		{
			// フラスタムカリングの実行
			{
				DrawIndexedInstancedIndirectArgs resetArgs = {};
				resetArgs.IndexCountPerInstance = mesh->indexCount;
				resetArgs.InstanceCount = 0;  // カリング後に設定される
				resetArgs.StartIndexLocation = 0;
				resetArgs.BaseVertexLocation = 0;
				resetArgs.StartInstanceLocation = 0;
				context->UpdateSubresource(m_indirectArgsBuffer.Get(), 0, nullptr, &resetArgs, 0, 0);

				// Compute Shader用の定数バッファを更新
				D3D11_MAPPED_SUBRESOURCE mapped;
				context->Map(m_computeConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
				ComputeConstants* constants = static_cast<ComputeConstants*>(mapped.pData);
				MakeFrustumData(testview, m_projection, *constants);
				constants->TotalInstanceCount = Count;
				constants->IndexCountPerInstance = mesh->indexCount;
				context->Unmap(m_computeConstantBuffer.Get(), 0);

				// Compute Shaderの設定
				context->CSSetShader(m_computeShader.Get(), nullptr, 0);
				context->CSSetConstantBuffers(0, 1, m_computeConstantBuffer.GetAddressOf());

				// リソースの設定
				context->CSSetShaderResources(0, 1, m_allInstanceSRV.GetAddressOf());
				ID3D11UnorderedAccessView* uavs[2] = { m_indirectArgsUAV.Get(), m_visibleInstanceUAV.Get() };
				context->CSSetUnorderedAccessViews(0, 2, uavs, nullptr);

				// Compute Shaderの実行
				UINT threadGroupCount = (Count + 63) / 64; // 64スレッドごとに1グループ
				context->Dispatch(threadGroupCount, 1, 1);

				// リソースのアンバインド
				ID3D11ShaderResourceView* nullSRV = nullptr;
				ID3D11UnorderedAccessView* nullUAVs[2] = { nullptr, nullptr };
				context->CSSetShaderResources(0, 1, &nullSRV);
				context->CSSetUnorderedAccessViews(0, 2, nullUAVs, nullptr);
			}

			// Compute Shaderの完了を確実にするため、頂点バッファをアンバインド
			ID3D11Buffer* nullBuffers[2] = { nullptr, nullptr };
			UINT nullStrides[2] = { 0, 0 };
			UINT nullOffsets[2] = { 0, 0 };
			context->IASetVertexBuffers(0, 2, nullBuffers, nullStrides, nullOffsets);

			// 描画パイプラインの設定
			ID3D11Buffer* pBuf[2] = { mesh->vertexBuffer.Get(), m_visibleInstanceBuffer.Get() };
			UINT strides[2] = { mesh->vertexStride, sizeof(Matrix) };
			UINT offsets[2] = { 0, 0 };

			context->IASetVertexBuffers(0, 2, pBuf, strides, offsets);
			context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			context->VSSetShader(m_testSet.vertexShader.Get(), nullptr, 0);
			context->PSSetShader(m_testSet.pixelShader.Get(), nullptr, 0);
			context->IASetInputLayout(m_testSet.inputLayout.Get());

			context->OMSetBlendState(states->Opaque(), nullptr, 0xffffffff);
			context->OMSetDepthStencilState(states->DepthDefault(), 0);

			context->PSSetShaderResources(0, 1, m_texture.GetAddressOf());
			ID3D11SamplerState* pSampler = states->LinearWrap();
			context->PSSetSamplers(0, 1, &pSampler);
			context->RSSetState(states->CullNone());

			// 定数バッファの設定
			CBuff sb;
			sb.World = Matrix::Identity;
			sb.View = view.Transpose();
			sb.Projection = m_projection.Transpose();

			Vector4 LightDir(0.5f, -1.0f, 0.5f, 0.0f);
			LightDir.Normalize();
			sb.LightDir = LightDir;
			sb.Diffuse = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
			sb.Emissive = Vector4(0.4f, 0.4f, 0.4f, 0);

			context->UpdateSubresource(m_testSet.cBuffer.Get(), 0, nullptr, &sb, 0, 0);
			ID3D11Buffer* pConstantBuffer = m_testSet.cBuffer.Get();
			context->VSSetConstantBuffers(0, 1, &pConstantBuffer);
			context->PSSetConstantBuffers(0, 1, &pConstantBuffer);

			// 間接描画の実行
			context->DrawIndexedInstancedIndirect(m_indirectArgsBuffer.Get(), 0);
		}
	}

	// デバッグ情報を「DebugString」で表示する
	auto debugString = m_commonResources->GetDebugString();
	debugString->AddString("Total Instances: %d", Count);
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

void PlayScene::MakeFrustumData(DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix projection, ComputeConstants& data)
{
	DirectX::BoundingFrustum::CreateFromMatrix(m_frustum, projection);
	DirectX::XMMATRIX invView = XMMatrixInverse(nullptr, view);
	m_frustum.Transform(m_frustum, invView);

	DirectX::SimpleMath::Vector3 corners[8];
	m_frustum.GetCorners(corners);

	// 視錐台の平面を計算（DirectXの左手座標系）
	data.FrustumPlanes[0] = CalculatePlane(corners[0], corners[1], corners[2]);  // 近平面
	data.FrustumPlanes[1] = CalculatePlane(corners[4], corners[7], corners[5]);  // 遠平面
	data.FrustumPlanes[2] = CalculatePlane(corners[0], corners[3], corners[7]);  // 左平面
	data.FrustumPlanes[3] = CalculatePlane(corners[1], corners[5], corners[6]);  // 右平面
	data.FrustumPlanes[4] = CalculatePlane(corners[3], corners[2], corners[6]);  // 上平面
	data.FrustumPlanes[5] = CalculatePlane(corners[0], corners[4], corners[5]);  // 下平面

	for (int i = 0; i < 8; i++)
	{
		data.FrustumCorners[i] = DirectX::SimpleMath::Vector4(corners[i].x, corners[i].y, corners[i].z, 0);
	}
}

DirectX::SimpleMath::Vector4 PlayScene::CalculatePlane(const DirectX::SimpleMath::Vector3& a, const DirectX::SimpleMath::Vector3& b, const DirectX::SimpleMath::Vector3& c)
{
	// 2つのベクトルを計算
	DirectX::SimpleMath::Vector3 v1 = b - a;
	DirectX::SimpleMath::Vector3 v2 = c - a;

	// 外積で法線を計算し、正規化
	DirectX::SimpleMath::Vector3 normal = v1.Cross(v2);
	normal.Normalize();

	// 平面の距離部分を計算
	float distance = -normal.Dot(a);

	return DirectX::SimpleMath::Vector4(normal.x, normal.y, normal.z, distance);
}