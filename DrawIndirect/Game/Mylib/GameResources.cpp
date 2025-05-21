// ================================================================ 
// ファイル名 : GameResources.h
// 作成者 : 景山碧天
// 説明 :  画像、モデルを管理
// ================================================================
#include"pch.h"
#include"GameResources.h"
#include"nlohmann/json.hpp"
#include <fstream>
#include <sstream>

/// <summary>
/// コンストラクタ
/// </summary>
GameResources::GameResources()
	:
	m_device{nullptr},
	m_models{},
	m_textures{}
{

}

/// <summary>
/// デストラクタ
/// </summary>
GameResources::~GameResources()
{
	m_models.clear();
	m_textures.clear();
}

/// <summary>
/// データの読み込み
/// </summary>
/// <param name="Pathnum">パスの番号</param>
void GameResources::LoadData(int Pathnum)
{
	//JSONファイル選択
	std::string file = "Resources/JSON/GameResources/Path" + std::to_string(Pathnum) + ".json";

	//読み込み
	std::ifstream ifs(file.c_str());
	if (ifs.good())
	{
		nlohmann::json m_json;
		ifs >> m_json;

		//モデルデータの読み込み
		for (auto itr = m_json["Model"].begin(); itr != m_json["Model"].end(); itr++)
		{
			std::string name = itr.key();
			std::string fileName = itr.value();
			LoadModels(name, fileName);
		}

		//画像データの読み込み
		for (auto itr = m_json["Texture"].begin(); itr != m_json["Texture"].end(); itr++)
		{
			std::string name = itr.key();
			std::string fileName = itr.value();
			LoadTexture(name, fileName);
		}

		//ファイルを閉じる
		ifs.close();
	}
}

/// <summary>
/// モデル取得
/// </summary>
/// <param name="name">名前</param>
/// <returns>モデル</returns>
DirectX::Model* GameResources::GetModel(std::string name)
{
	auto itr = m_models.find(name);
	if (itr != m_models.end())
	{
		return m_models[name].get();
	}
	return nullptr;
}

/// <summary>
/// 画像取得
/// </summary>
/// <param name="name">名前</param>
/// <returns>画像</returns>
ID3D11ShaderResourceView* GameResources::GetTexture(std::string name)
{
	auto itr = m_textures.find(name);
	if (itr != m_textures.end())
	{
		return m_textures[name].Get();
	}
	return nullptr;
}

/// <summary>
/// モデルの作成
/// </summary>
/// <param name="name">登録名</param>
/// <param name="fileName">モデルのファイル名</param>
void GameResources::LoadModels(std::string name, std::string fileName)
{
	std::string folderPath = "Resources/Models/";

	std::unique_ptr<DirectX::EffectFactory> fx = std::make_unique<DirectX::EffectFactory>(m_device);
	std::wstring myWideString = std::wstring(folderPath.begin(), folderPath.end());
	const wchar_t* folder = myWideString.c_str();
	fx->SetDirectory(folder);

	//	モデルの作成
	folderPath += fileName;
	myWideString = std::wstring(folderPath.begin(), folderPath.end());
	folder = myWideString.c_str();
	std::unique_ptr<DirectX::Model> model = DirectX::Model::CreateFromCMO(m_device, folder, *fx);

	//挿入
	m_models.insert({ name, std::move(model) });
}

/// <summary>
/// 画像の作成
/// </summary>
/// <param name="name">登録名</param>
/// <param name="fileName">画像のファイル名</param>
void GameResources::LoadTexture(std::string name, std::string fileName)
{
	std::string folderPath = "Resources/Textures/" + fileName;

	std::wstring myWideString = std::wstring(folderPath.begin(), folderPath.end());
	const wchar_t* file = myWideString.c_str();

	// 画像の作成
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> image;
	DX::ThrowIfFailed(
		DirectX::CreateWICTextureFromFile(
			m_device,
			file,
			nullptr,
			image.ReleaseAndGetAddressOf(),
			0
		)
	);

	//挿入
	m_textures.insert({ name,image });
}






