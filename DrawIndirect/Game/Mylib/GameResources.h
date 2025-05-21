// ================================================================ 
// ファイル名 : GameResources.h
// 作成者 : 景山碧天
// 説明 :  画像、モデルを管理
// ================================================================
#pragma once
#include"Singleton.h"

class GameResources:public Singleton<GameResources> 
{
	friend class Singleton<GameResources>;
private:
	GameResources();
private:
	//デバイス
	ID3D11Device* m_device;
	//モデル情報
	std::unordered_map<std::string, std::unique_ptr<DirectX::Model>> m_models;
	//画像情報
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_textures;
public:
	~GameResources();
	//デバイスセット
	void SetDevice(ID3D11Device* device) { m_device = device; }
	//データ読み込み
	void LoadData(int Pathnum = 1);
	//モデル取り出し
	DirectX::Model* GetModel(std::string name);
	//画像取り出し
	ID3D11ShaderResourceView* GetTexture(std::string name);
private:
	//モデル読み込み
	void LoadModels(std::string name, std::string fileName);
	//画像読み込み
	void LoadTexture(std::string name, std::string fileName);
};