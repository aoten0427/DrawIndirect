#pragma once
#include"Game/Mylib/ShaderManager.h"

class Model3D
{
private:
	struct CBuff
	{
		DirectX::SimpleMath::Matrix mat;
	};
private:
	//モデルデータ
	DirectX::Model* m_model;
	//テクスチャ
	ID3D11ShaderResourceView* m_texture;
	//シェーダー
	ShaderSet m_shaderSet;
	//描画方法
	bool m_isUseShader;

	DirectX::SimpleMath::Matrix m_matrix;
public:
	Model3D(bool useShader = false);
	~Model3D() = default;

	void Initialize(ID3D11Device* device);
	void LoadModel(std::string filename);
	void LoadTexture(std::string filename);
	void Render(ID3D11DeviceContext* context,
		DirectX::DX11::CommonStates* states,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& projection);

	void SetMatrix(DirectX::SimpleMath::Matrix& mat) { m_matrix = mat; }

	void SetShader(ShaderSet& set);
};