#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "texture.h"
//textureのプーリング

std::unordered_map<std::string, ID3D11ShaderResourceView*>  Texture::m_TexturePool;

ID3D11ShaderResourceView* Texture::Load(const char* FileName)
{
	if (m_TexturePool.count(FileName) > 0)
	{
		return m_TexturePool[FileName];
	}

	wchar_t wFileName[512];
	mbstowcs(wFileName, FileName, strlen(FileName) + 1);

	//texture yomikomi
	TexMetadata metadata;
	ScratchImage image;
	ID3D11ShaderResourceView* texture;
	LoadFromWICFile(wFileName, WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(Renderer::GetDevice(),
		image.GetImages(), image.GetImageCount(), metadata, &texture);
	assert(texture);

	if (texture) {
		std::string name = "Texture: " + std::string(FileName);
		texture->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.c_str());
	}

	m_TexturePool[FileName] = texture;

	return texture;
}
void Texture::UnloadAll()
{
	// マップの中身を全走査して Release する
	for (auto& pair : m_TexturePool)
	{
		// pair.first  が ファイル名 (key)
		// pair.second が SRVのポインタ (value)
		if (pair.second)
		{
			pair.second->Release();
			pair.second = nullptr;
		}
	}

	// 最後にマップ自体を空にする
	m_TexturePool.clear();
}
