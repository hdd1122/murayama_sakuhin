
#include "main.h"
#include "renderer.h"
#include "modelRenderer.h"


#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

//
// objファイルからのモデル描画まで
//

std::unordered_map<std::string, MODEL*> ModelRenderer::m_ModelPool;

void ModelRenderer::Draw()
{

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_Model->VertexBuffer, &stride, &offset);

	// インデックスバッファ設定
	Renderer::GetDeviceContext()->IASetIndexBuffer(m_Model->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// プリミティブトポロジ設定
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// --- 透過マテリアルが含まれているかチェック ---
	bool isTransparent = false;
	for (unsigned int i = 0; i < m_Model->SubsetNum; i++)
	{
		if (m_Model->SubsetArray[i].Material.Material.Diffuse.w < 0.99f)
		{
			isTransparent = true;
			break;
		}
	}

	//ブレンドステート設定　これsetStateに配置した方が
	Renderer::SetATCEnable(isTransparent);  // 半透明ON/OFF



	for( unsigned int i = 0; i < m_Model->SubsetNum; i++ )
	{
		// マテリアル設定
		Renderer::SetMaterial(m_Model->SubsetArray[i].Material.Material );


		// テクスチャ設定
		if(m_Model->SubsetArray[i].Material.Texture)
			Renderer::GetDeviceContext()->PSSetShaderResources( 0, 1, &m_Model->SubsetArray[i].Material.Texture );

		
		// ポリゴン描画
		Renderer::GetDeviceContext()->DrawIndexed(m_Model->SubsetArray[i].IndexNum, m_Model->SubsetArray[i].StartIndex, 0 );

	}

}

void ModelRenderer::DrawSetBloom()
{

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_Model->VertexBuffer, &stride, &offset);

	// インデックスバッファ設定
	Renderer::GetDeviceContext()->IASetIndexBuffer(m_Model->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// プリミティブトポロジ設定
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// --- 透過マテリアルが含まれているかチェック ---
	bool isTransparent = false;
	for (unsigned int i = 0; i < m_Model->SubsetNum; i++)
	{
		if (m_Model->SubsetArray[i].Material.Material.Diffuse.w < 0.99f)
		{
			isTransparent = true;
			break;
		}
	}

	//ブレンドステート設定　これsetStateに配置した方が
	Renderer::SetATCEnable(isTransparent);  // 半透明ON/OFF



	for (unsigned int i = 0; i < m_Model->SubsetNum; i++)
	{
		// マテリアル設定
		Renderer::SetMaterial(m_Model->SubsetArray[i].Material.Material);

		//マテリアルのエミッション経由にすると変更箇所が増えるから既存の形で使用する
		BloomColor bc;
		bc.bloomColor = m_Model->SubsetArray[i].Material.Material.Emission;
		Renderer::UpdateBloomColor(bc);


		if (m_Model->SubsetArray[i].Material.EmissionTexture)
		{
			Renderer::GetDeviceContext()->PSSetShaderResources(1, 1, &m_Model->SubsetArray[i].Material.EmissionTexture);
		}
		else
		{
			// テクスチャがないときは、作ったダミー黒テクスチャをセット
			ID3D11ShaderResourceView* pDummy = Renderer::GetDummyTextureSRV();
			Renderer::GetDeviceContext()->PSSetShaderResources(1, 1, &pDummy);
		}


		// テクスチャ設定
		if (m_Model->SubsetArray[i].Material.Texture)
			Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Model->SubsetArray[i].Material.Texture);



		// ポリゴン描画
		Renderer::GetDeviceContext()->DrawIndexed(m_Model->SubsetArray[i].IndexNum, m_Model->SubsetArray[i].StartIndex, 0);

	}

}

unsigned int ModelRenderer::GetIndexCount() const
{
	if (!m_Model || m_Model->SubsetNum == 0) return 0;

	// もし複数サブセットがあるなら、合計を返すか、サブセットごとの処理が必要だが、
	// ポイントライト用の簡易モデルならこれで
	return m_Model->SubsetArray[0].IndexNum;
}

void ModelRenderer::BindBuffers(ID3D11DeviceContext* context)
{
	if (!m_Model) return;

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_Model->VertexBuffer, &stride, &offset);

	// インデックスバッファ設定
	context->IASetIndexBuffer(m_Model->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// プリミティブトポロジー設定
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


void ModelRenderer::DrawOitAccumulation()
{

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_Model->VertexBuffer, &stride, &offset);

	// インデックスバッファ設定
	Renderer::GetDeviceContext()->IASetIndexBuffer(m_Model->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// プリミティブトポロジ設定
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



	for (unsigned int i = 0; i < m_Model->SubsetNum; i++)
	{
		//描画前にオブジェクトが設定するためモデルから読み込んだデータは使わない

		//// マテリアル設定
		//Renderer::SetMaterial(m_Model->SubsetArray[i].Material.Material);

		//// テクスチャ設定
		//if (m_Model->SubsetArray[i].Material.Texture)
		//	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Model->SubsetArray[i].Material.Texture);



		// ポリゴン描画
		Renderer::GetDeviceContext()->DrawIndexed(m_Model->SubsetArray[i].IndexNum, m_Model->SubsetArray[i].StartIndex, 0);

	}

}




void ModelRenderer::Preload(const char *FileName)
{
	if (m_ModelPool.count(FileName) > 0)
	{
		return;
	}

	MODEL* model = new MODEL;
	LoadModel(FileName, model);

	m_ModelPool[FileName] = model;

	
}

void ModelRenderer::UnloadAll()
{
	
	for (auto& pair : m_ModelPool)
	{
		MODEL* model = pair.second;
		if (!model) continue;

		// 頂点バッファの解放
		if (model->VertexBuffer) {
			model->VertexBuffer->Release();
			model->VertexBuffer = nullptr;
		}

		// インデックスバッファの解放
		if (model->IndexBuffer) {
			model->IndexBuffer->Release();
			model->IndexBuffer = nullptr;
		}

		// 各サブセットのリソース解放
		if (model->SubsetArray) {
			for (unsigned int i = 0; i < model->SubsetNum; i++) {
				if (model->SubsetArray[i].Material.Texture) {
					model->SubsetArray[i].Material.Texture->Release();
					model->SubsetArray[i].Material.Texture = nullptr;
				}
				if (model->SubsetArray[i].Material.EmissionTexture) {
					model->SubsetArray[i].Material.EmissionTexture->Release();
					model->SubsetArray[i].Material.EmissionTexture = nullptr;
				}
			}
			// 配列自体の削除
			delete[] model->SubsetArray;
		}

		// MODEL構造体自体の削除
		delete model;
	}

	// マップを空にする
	m_ModelPool.clear();
}


void ModelRenderer::Load(const char *FileName)
{
	if (m_ModelPool.count(FileName) > 0)
	{
		m_Model = m_ModelPool[FileName];
		return;
	}

	m_Model = new MODEL;
	LoadModel(FileName, m_Model);

	m_ModelPool[FileName] = m_Model;


}

void ModelRenderer::LoadModel( const char *FileName, MODEL *Model)
{

	MODEL_OBJ modelObj;
	LoadObj( FileName, &modelObj );



	// 頂点バッファ生成
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( VERTEX_3D ) * modelObj.VertexNum;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory( &sd, sizeof(sd) );
		sd.pSysMem = modelObj.VertexArray;

		Renderer::GetDevice()->CreateBuffer( &bd, &sd, &Model->VertexBuffer );
	}


	// インデックスバッファ生成
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( unsigned int ) * modelObj.IndexNum;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory( &sd, sizeof(sd) );
		sd.pSysMem = modelObj.IndexArray;

		Renderer::GetDevice()->CreateBuffer( &bd, &sd, &Model->IndexBuffer );
	}

	// サブセット設定
	{
		Model->SubsetArray = new SUBSET[ modelObj.SubsetNum ];
		Model->SubsetNum = modelObj.SubsetNum;

		for( unsigned int i = 0; i < modelObj.SubsetNum; i++ )
		{
			Model->SubsetArray[i].StartIndex = modelObj.SubsetArray[i].StartIndex;
			Model->SubsetArray[i].IndexNum = modelObj.SubsetArray[i].IndexNum;

			Model->SubsetArray[i].Material.Material = modelObj.SubsetArray[i].Material.Material;

			Model->SubsetArray[i].Material.Texture = nullptr;
			Model->SubsetArray[i].Material.EmissionTexture = nullptr;


			// テクスチャ読み込み
			TexMetadata metadata;
			ScratchImage image;
			wchar_t wc[256];
			mbstowcs(wc, modelObj.SubsetArray[i].Material.TextureName, sizeof(wc));
			LoadFromWICFile(wc, WIC_FLAGS_NONE, &metadata, image);
			CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &Model->SubsetArray[i].Material.Texture);

			if (Model->SubsetArray[i].Material.Texture)
				Model->SubsetArray[i].Material.Material.TextureEnable = true;
			else
				Model->SubsetArray[i].Material.Material.TextureEnable = false;

			//エミッションテクスチャ読み込み	
			{
				TexMetadata metaEmi;
				ScratchImage imgEmi;
				wchar_t wcEmi[256];
				mbstowcs(wcEmi, modelObj.SubsetArray[i].Material.EmissionTextureName, sizeof(wcEmi));

				// ファイル名がある場合のみ読み込む
				if (wcslen(wcEmi) > 0)
				{
					HRESULT hr = LoadFromWICFile(wcEmi, WIC_FLAGS_NONE, &metaEmi, imgEmi);
					if (SUCCEEDED(hr))
					{
						CreateShaderResourceView(Renderer::GetDevice(),
							imgEmi.GetImages(), imgEmi.GetImageCount(), metaEmi,
							&Model->SubsetArray[i].Material.EmissionTexture);
					}
				}
			}

		}
	}

	delete[] modelObj.VertexArray;
	delete[] modelObj.IndexArray;
	delete[] modelObj.SubsetArray;

}






//モデル読込////////////////////////////////////////////
void ModelRenderer::LoadObj( const char *FileName, MODEL_OBJ *ModelObj )
{

	char dir[MAX_PATH];
	strcpy (dir, FileName );
	PathRemoveFileSpec(dir);





	XMFLOAT3* positionArray;
	XMFLOAT3* normalArray;
	XMFLOAT2* texcoordArray;

	unsigned int	positionNum = 0;
	unsigned int	normalNum = 0;
	unsigned int	texcoordNum = 0;
	unsigned int	vertexNum = 0;
	unsigned int	indexNum = 0;
	unsigned int	in = 0;
	unsigned int	subsetNum = 0;

	std::vector<MODEL_MATERIAL> materialArray;
	unsigned int	materialNum = 0;

	char str[256];
	char *s;
	char c;


	FILE *file;
	file = fopen( FileName, "rt" );
	assert(file);



	//要素数カウント
	while( true )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;

		if( strcmp( str, "v" ) == 0 )
		{
			positionNum++;
		}
		else if( strcmp( str, "vn" ) == 0 )
		{
			normalNum++;
		}
		else if( strcmp( str, "vt" ) == 0 )
		{
			texcoordNum++;
		}
		else if( strcmp( str, "usemtl" ) == 0 )
		{
			subsetNum++;
		}
		else if( strcmp( str, "f" ) == 0 )
		{
			in = 0;

			do
			{
				fscanf( file, "%s", str );
				vertexNum++;
				in++;
				c = fgetc( file );
			}
			while( c != '\n' && c!= '\r' );

			//四角は三角に分割
			if( in == 4 )
				in = 6;

			indexNum += in;
		}
	}


	//メモリ確保
	positionArray = new XMFLOAT3[ positionNum ];
	normalArray = new XMFLOAT3[ normalNum ];
	texcoordArray = new XMFLOAT2[ texcoordNum ];


	ModelObj->VertexArray = new VERTEX_3D[ vertexNum ];
	ModelObj->VertexNum = vertexNum;

	ModelObj->IndexArray = new unsigned int[ indexNum ];
	ModelObj->IndexNum = indexNum;

	ModelObj->SubsetArray = new SUBSET[ subsetNum ];
	ModelObj->SubsetNum = subsetNum;




	//要素読込
	XMFLOAT3 *position = positionArray;
	XMFLOAT3 *normal = normalArray;
	XMFLOAT2 *texcoord = texcoordArray;

	unsigned int vc = 0;
	unsigned int ic = 0;
	unsigned int sc = 0;


	fseek( file, 0, SEEK_SET );

	while( true )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;

		if( strcmp( str, "mtllib" ) == 0 )
		{
			//マテリアルファイル
			fscanf( file, "%s", str );

			char path[256];
			strcpy( path, dir );
			strcat( path, "\\" );
			strcat( path, str );

			LoadMaterial( path, materialArray);
		}
		else if( strcmp( str, "o" ) == 0 )
		{
			//オブジェクト名
			fscanf( file, "%s", str );
		}
		else if( strcmp( str, "v" ) == 0 )
		{
			//頂点座標
			fscanf( file, "%f", &position->x );
			fscanf( file, "%f", &position->y );
			fscanf( file, "%f", &position->z );
			position++;
		}
		else if( strcmp( str, "vn" ) == 0 )
		{
			//法線
			fscanf( file, "%f", &normal->x );
			fscanf( file, "%f", &normal->y );
			fscanf( file, "%f", &normal->z );
			normal++;
		}
		else if( strcmp( str, "vt" ) == 0 )
		{
			//テクスチャ座標
			fscanf( file, "%f", &texcoord->x );
			fscanf( file, "%f", &texcoord->y );
			texcoord->x = 1.0f - texcoord->x;
			texcoord->y = 1.0f - texcoord->y;
			texcoord++;
		}
		else if( strcmp( str, "usemtl" ) == 0 )
		{
			//マテリアル
			fscanf( file, "%s", str );

			if( sc != 0 )
				ModelObj->SubsetArray[ sc - 1 ].IndexNum = ic - ModelObj->SubsetArray[ sc - 1 ].StartIndex;

			ModelObj->SubsetArray[ sc ].StartIndex = ic;


			for( unsigned int i = 0; i < materialArray.size(); i++ )
			{
				if( strcmp( str, materialArray[i].Name ) == 0 )
				{
					ModelObj->SubsetArray[ sc ].Material.Material = materialArray[i].Material;
					strcpy( ModelObj->SubsetArray[ sc ].Material.TextureName, materialArray[i].TextureName );
					strcpy(ModelObj->SubsetArray[sc].Material.EmissionTextureName, materialArray[i].EmissionTextureName);
					strcpy( ModelObj->SubsetArray[ sc ].Material.Name, materialArray[i].Name );

					break;
				}
			}

			sc++;
			
		}
		else if( strcmp( str, "f" ) == 0 )
		{
			//面
			in = 0;

			do
			{
				fscanf( file, "%s", str );

				s = strtok( str, "/" );	
				ModelObj->VertexArray[vc].Position = positionArray[ atoi( s ) - 1 ];
				if( s[ strlen( s ) + 1 ] != '/' )
				{
					//テクスチャ座標が存在しない場合もある
					s = strtok( nullptr, "/" );
					ModelObj->VertexArray[vc].TexCoord = texcoordArray[ atoi( s ) - 1 ];
				}
				s = strtok( nullptr, "/" );	
				ModelObj->VertexArray[vc].Normal = normalArray[ atoi( s ) - 1 ];

				ModelObj->VertexArray[vc].Diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );

				ModelObj->IndexArray[ic] = vc;
				ic++;
				vc++;

				in++;
				c = fgetc( file );
			}
			while( c != '\n' && c != '\r' );

			//四角は三角に分割
			if( in == 4 )
			{
				ModelObj->IndexArray[ic] = vc - 4;
				ic++;
				ModelObj->IndexArray[ic] = vc - 2;
				ic++;
			}
		}
	}


	if( sc != 0 )
		ModelObj->SubsetArray[ sc - 1 ].IndexNum = ic - ModelObj->SubsetArray[ sc - 1 ].StartIndex;


	fclose( file );


	delete[] positionArray;
	delete[] normalArray;
	delete[] texcoordArray;
	
}




//マテリアル読み込み
void ModelRenderer::LoadMaterial(const char* FileName, std::vector<MODEL_MATERIAL>& materialArray)
{
	char dir[MAX_PATH];
	strcpy(dir, FileName);
	PathRemoveFileSpec(dir);

	char str[256];

	FILE* file;
	file = fopen(FileName, "rt");
	assert(file);

	while (true)
	{
		fscanf(file, "%s", str);

		if (feof(file) != 0)
			break;

		if (strcmp(str, "newmtl") == 0)
		{
			// 新しいマテリアルを作成して初期化
			MODEL_MATERIAL newMat;
			memset(&newMat, 0, sizeof(MODEL_MATERIAL)); // ゼロクリア

			fscanf(file, "%s", newMat.Name);
			strcpy(newMat.TextureName, "");

			strcpy(newMat.EmissionTextureName, "");
			newMat.EmissionTexture = nullptr;

			newMat.Material.Emission.x = 0.0f;
			newMat.Material.Emission.y = 0.0f;
			newMat.Material.Emission.z = 0.0f;
			newMat.Material.Emission.w = 0.0f;

			// ★修正: vectorの末尾に追加する
			materialArray.push_back(newMat);
		}
		//vectorに要素が1つ以上ある場合のみパラメータを設定する
		else if (!materialArray.empty())
		{
			
			MODEL_MATERIAL& curMat = materialArray.back();

			if (strcmp(str, "Ka") == 0)
			{
				//アンビエント
				fscanf(file, "%f", &curMat.Material.Ambient.x);
				fscanf(file, "%f", &curMat.Material.Ambient.y);
				fscanf(file, "%f", &curMat.Material.Ambient.z);
				curMat.Material.Ambient.w = 1.0f;
			}
			else if (strcmp(str, "Kd") == 0)
			{
				//ディフューズ
				fscanf(file, "%f", &curMat.Material.Diffuse.x);
				fscanf(file, "%f", &curMat.Material.Diffuse.y);
				fscanf(file, "%f", &curMat.Material.Diffuse.z);
				curMat.Material.Diffuse.w = 1.0f;
			}
			else if (strcmp(str, "Ks") == 0)
			{
				//スペキュラ
				fscanf(file, "%f", &curMat.Material.Specular.x);
				fscanf(file, "%f", &curMat.Material.Specular.y);
				fscanf(file, "%f", &curMat.Material.Specular.z);
				curMat.Material.Specular.w = 1.0f;
			}
			else if (strcmp(str, "Ns") == 0)
			{
				//スペキュラ強度
				fscanf(file, "%f", &curMat.Material.Shininess);
			}
			else if (strcmp(str, "d") == 0)
			{
				//アルファ
				fscanf(file, "%f", &curMat.Material.Diffuse.w);
			}
			else if (strcmp(str, "map_Kd") == 0)
			{
				//テクスチャ
				fscanf(file, "%s", str);

				char path[256];
				strcpy(path, dir);
				strcat(path, "\\");
				strcat(path, str);

				strcat(curMat.TextureName, path);
			}
			else if (strcmp(str, "Ke") == 0)
			{
				//エミッション
				fscanf(file, "%f", &curMat.Material.Emission.x);
				fscanf(file, "%f", &curMat.Material.Emission.y);
				fscanf(file, "%f", &curMat.Material.Emission.z);
			}
			else if (strcmp(str, "Ni") == 0)
			{
				//Niの屈折率はエミッションの強さにする
				fscanf(file, "%f", &curMat.Material.Emission.w);
			}
			//エミッションテクスチャ名の読み込み
			else if (strcmp(str, "map_Ke") == 0)
			{
				fscanf(file, "%s", str);
				char path[256];
				strcpy(path, dir);
				strcat(path, "\\");
				strcat(path, str);

				strcpy(curMat.EmissionTextureName, path);
			}
		}
	}

	fclose(file);

}

