#include"pch.h"
#include"Model3D.h"
#include"DeviceResources.h"
#include"Game/Mylib/GameResources.h"

Model3D::Model3D(bool useshader)
	:
	m_model{},
	m_texture{},
	m_isUseShader{useshader}
{

}

void Model3D::Initialize(ID3D11Device* device)
{



}

void Model3D::LoadModel(std::string filename)
{
	m_model = GameResources::GetInstance()->GetModel(filename);
}

void Model3D::LoadTexture(std::string filename)
{
	m_texture = GameResources::GetInstance()->GetTexture(filename);
}



void Model3D::Render(ID3D11DeviceContext* context, DirectX::DX11::CommonStates* states, const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& projection)
{
	if (!m_isUseShader)
	{
		m_model->Draw(context, *states, m_matrix, view, projection);
	}
	else
	{
		for (auto& meshes : m_model->meshes)
		{
			for (auto& mesh : meshes->meshParts)
			{
				UINT stride = sizeof(DirectX::VertexPositionNormalTangentColorTexture); // VertexTypeは実際の頂点構造体に合わせる
				UINT offset = 0;
				context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(), &stride, &offset);
				//インデックスバッファのセット
				context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
				//描画方法（3角形）D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
				context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				//シェーダの設定
				context->VSSetShader(m_shaderSet.vertexShader.Get(), nullptr, 0);
				context->PSSetShader(m_shaderSet.pixelShader.Get(), nullptr, 0);
				//インプットレイアウトの設定
				context->IASetInputLayout(m_shaderSet.inputLayout.Get());

				//ブレンドステート
				//透明処理しない
				context->OMSetBlendState(states->Opaque(), nullptr, 0xffffffff);
				//デプスステンシルステート
				context->OMSetDepthStencilState(states->DepthDefault(), 0);
				//テクスチャとサンプラーの設定
				ID3D11ShaderResourceView* pNull[1] = { 0 };
				context->PSSetShaderResources(0, 1, &m_texture);
				ID3D11SamplerState* pSampler = states->LinearWrap();;
				context->PSSetSamplers(0, 1, &pSampler);
				//ラスタライザステート（表面描画）
				context->RSSetState(states->CullNone());

				CBuff cb;
				DirectX::SimpleMath::Matrix world = m_matrix * view * projection;
				cb.mat = world.Transpose();

				//コンスタントバッファの更新
				context->UpdateSubresource(m_shaderSet.cBuffer.Get(), 0, nullptr, &cb, 0, 0);
				//コンスタントバッファの設定
				ID3D11Buffer* pConstantBuffer = m_shaderSet.cBuffer.Get();
				ID3D11Buffer* pNullConstantBuffer = nullptr;
				//頂点シェーダに渡す
				context->VSSetConstantBuffers(0, 1, &pConstantBuffer);
				//ピクセルシェーダに渡す
				context->PSSetConstantBuffers(0, 1, &pConstantBuffer);
				//描画
				// インデックスバッファを使用した描画に変更
				context->DrawIndexed(mesh->indexCount, 0, 0);  // Drawの代わりにDrawIndexedを使用
			}
		}
	}



	

}

void Model3D::SetShader(ShaderSet& set)
{
	m_shaderSet.vertexShader = set.vertexShader;
	m_shaderSet.pixelShader = set.pixelShader;
	m_shaderSet.inputLayout = set.inputLayout;
	m_shaderSet.cBuffer = set.cBuffer;
}
