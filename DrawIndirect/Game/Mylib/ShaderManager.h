// ================================================================ 
// ファイル名 : ShaderManager.h
// 作成者 : 景山碧天
// 説明 :  シェーダー関連の生成を行う
// ================================================================

#pragma once
#include"ReadData.h"

/// <summary>
/// モデル用インプットレイアウト
/// </summary>
const std::vector<D3D11_INPUT_ELEMENT_DESC> MODEL_INPUT_LAYOUT =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

/// <summary>
/// 2D用インプットレイアウト
/// </summary>
const std::vector<D3D11_INPUT_ELEMENT_DESC> NOMAL_INPUT_LAYOUT =
{
	 { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

enum class ShaderType {
	VertexShader,
	PixelShader,
	GeometryShader
};

/// <summary>
/// シェーダ使用時にまとまり
/// </summary>
struct ShaderSet
{
	//定数バッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer>	cBuffer;
	//	入力レイアウト
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	//	頂点シェーダ
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	//	ピクセルシェーダ
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
};


class ShaderManager
{
public:
	
	struct ModelCbuff
	{
		DirectX::SimpleMath::Matrix		matWorld;
		DirectX::SimpleMath::Matrix		matView;
		DirectX::SimpleMath::Matrix		matProj;
		DirectX::SimpleMath::Vector4	Diffuse;
	};
public:
	/// <summary>
	/// 頂点シェーダ作成
	/// </summary>
	/// <param name="device"></param>
	/// <param name="fileName"></param>
	/// <returns></returns>
	static Microsoft::WRL::ComPtr<ID3D11VertexShader> CreateVSShader(ID3D11Device* device, std::string fileName)
	{
		fileName = "Resources/Shaders/" + fileName;
		std::wstring myWideString = std::wstring(fileName.begin(), fileName.end());
		const wchar_t* folder = myWideString.c_str();

		std::vector<uint8_t> vs = DX::ReadData(folder);

		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
		//	頂点シェーダ作成
		if (FAILED(device->CreateVertexShader(vs.data(), vs.size(), NULL, vertexShader.ReleaseAndGetAddressOf())))
		{//	エラー
			MessageBox(0, L"CreateVertexShader Failed.", NULL, MB_OK);
		}

		return vertexShader;
	}

	/// <summary>
	/// ジオメトリシェーダ作成
	/// </summary>
	/// <param name="device"></param>
	/// <param name="fileName"></param>
	/// <returns></returns>
	static Microsoft::WRL::ComPtr<ID3D11GeometryShader> CreateGSShader(ID3D11Device* device, std::string fileName)
	{
		fileName = "Resources/Shaders/" + fileName;
		std::wstring myWideString = std::wstring(fileName.begin(), fileName.end());
		const wchar_t* folder = myWideString.c_str();

		std::vector<uint8_t> gs = DX::ReadData(folder);

		Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometryShader;
		//	頂点シェーダ作成
		if (FAILED(device->CreateGeometryShader(gs.data(), gs.size(), NULL, geometryShader.ReleaseAndGetAddressOf())))
		{//	エラー
			MessageBox(0, L"CreateVertexShader Failed.", NULL, MB_OK);
		}

		return geometryShader;
	}

	/// <summary>
	/// ピクセルシェーダー作成
	/// </summary>
	/// <param name="device"></param>
	/// <param name="fileName"></param>
	/// <returns></returns>
	static Microsoft::WRL::ComPtr<ID3D11PixelShader> CreatePSShader(ID3D11Device* device, std::string fileName)
	{
		fileName = "Resources/Shaders/" + fileName;
		std::wstring myWideString = std::wstring(fileName.begin(), fileName.end());
		const wchar_t* folder = myWideString.c_str();

		std::vector<uint8_t> ps = DX::ReadData(folder);

		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
		//	頂点シェーダ作成
		if (FAILED(device->CreatePixelShader(ps.data(), ps.size(), NULL, pixelShader.ReleaseAndGetAddressOf())))
		{//	エラー
			MessageBox(0, L"CreateVertexShader Failed.", NULL, MB_OK);
		}

		return pixelShader;
	}

	static Microsoft::WRL::ComPtr<ID3D11ComputeShader> CreateCSShader(ID3D11Device* device, std::string fileName)
	{
		fileName = "Resources/Shaders/" + fileName;
		std::wstring myWideString = std::wstring(fileName.begin(), fileName.end());
		const wchar_t* folder = myWideString.c_str();

		std::vector<uint8_t> cs = DX::ReadData(folder);

		Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader;
		//	頂点シェーダ作成
		if (FAILED(device->CreateComputeShader(cs.data(), cs.size(), NULL, computeShader.ReleaseAndGetAddressOf())))
		{//	エラー
			MessageBox(0, L"CreateComputeShader Failed.", NULL, MB_OK);
		}

		return computeShader;
	}

	/// <summary>
	/// 定数バッファ作成
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="device"></param>
	/// <returns></returns>
	template<typename T>
	static	Microsoft::WRL::ComPtr<ID3D11Buffer> CreateConstantBuffer(ID3D11Device* device,
		D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
		D3D11_BIND_FLAG bindflag = D3D11_BIND_CONSTANT_BUFFER
	) {
		Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;


		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = usage;
		bd.ByteWidth = sizeof(T);
		bd.BindFlags = bindflag;
		bd.CPUAccessFlags = 0;
		device->CreateBuffer(&bd, nullptr, &constantBuffer);

		return constantBuffer;
	}

	//定数バッファのバインド
	static	void BindConstantBuffer(ID3D11DeviceContext* context, ID3D11Buffer* buffer, UINT slot, ShaderType shaderType) {
		switch (shaderType) {
		case ShaderType::VertexShader:
			context->VSSetConstantBuffers(slot, 1, &buffer);
			break;
		case ShaderType::PixelShader:
			context->PSSetConstantBuffers(slot, 1, &buffer);
			break;
		case ShaderType::GeometryShader:
			context->GSSetConstantBuffers(slot, 1, &buffer);
		}
	}

	/// <summary>
	/// インプットレイアウト作成
	/// </summary>
	/// <param name="device"></param>
	/// <param name="layoutDesc"></param>
	/// <param name="fileName"></param>
	/// <returns></returns>
	static Microsoft::WRL::ComPtr<ID3D11InputLayout> CreateInputLayout(
		ID3D11Device* device,
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& layoutDesc,
		std::string fileName)
	{
		fileName = "Resources/Shaders/" + fileName;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

		std::wstring myWideString = std::wstring(fileName.begin(), fileName.end());
		const wchar_t* folder = myWideString.c_str();

		std::vector<uint8_t> vs = DX::ReadData(folder);

		device->CreateInputLayout(
			layoutDesc.data(),
			static_cast<UINT>(layoutDesc.size()),
			vs.data(),
			vs.size(),
			&inputLayout);


		return inputLayout;
	}

	static void SetShaders(ID3D11DeviceContext* context, ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader)
	{
		context->VSSetShader(vertexShader, nullptr, 0);
		context->PSSetShader(pixelShader, nullptr, 0);
	}

	static void SetShaders(ID3D11DeviceContext* context, ID3D11VertexShader* vertexShader,
		ID3D11GeometryShader* geometyShader,
		ID3D11PixelShader* pixelShader)
	{
		context->VSSetShader(vertexShader, nullptr, 0);
		context->GSSetShader(geometyShader, nullptr, 0);
		context->PSSetShader(pixelShader, nullptr, 0);
	}
};

