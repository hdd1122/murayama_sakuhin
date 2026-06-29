
#include "main.h"
#include "renderer.h"
#include "meshField.h"
#include "input.h"
#include "texture.h"

# include <string>
# include <fstream>
# include <sstream>

//

// 値を0.0から1.0の範囲に正規化する関数
float normalize(float value) {
	if (value >= 3.0f) {
		return 1.0f;
	}
	else if (value <= 0.0f) {
		return 0.0f;
	}
	else {
		return value / 3.0f;
	}
}

void MeshField::Init()
{
	//AddComponent<Shader>()->Load("shader\\vertexLightingVS.cso", "shader\\vertexLightingPS.cso");


	// 頂点バッファ生成
	{
		for (int x = 0; x <= 20; x++)
		{
			for (int z = 0; z <= 20; z++)
			{
				float y = rand() % 7;
				if (x > 5 && x<14)
					y = 0;
				
				m_Vertex[x][z].Position = XMFLOAT3((x - 10) * 5.0f, y, (z - 10) * -5.0f);
				m_Vertex[x][z].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);//法線ベクトル
				m_Vertex[x][z].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, normalize(m_Vertex[x][z].Position.y));
				m_Vertex[x][z].TexCoord = XMFLOAT2(x * 0.5f, z * 0.5f);
			}
		}
		reLoad();
		for (int x = 0; x <= 20; x++)
		{
			for (int z = 0; z <= 20; z++)
			{
				m_Vertex[x][z].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, normalize(m_Vertex[x][z].Position.y));
			}
		}

		for (int x = 1; x < 20; x++)
		{
			for (int z = 1; z < 20; z++)
			{
				Vector3 vx, vz, vn;
				vx.x = m_Vertex[x + 1][z].Position.x - m_Vertex[x - 1][z].Position.x;
				vx.y = m_Vertex[x + 1][z].Position.y - m_Vertex[x - 1][z].Position.y;
				vx.z = m_Vertex[x + 1][z].Position.z - m_Vertex[x - 1][z].Position.z;

				vz.x = m_Vertex[x][z - 1].Position.x - m_Vertex[x][z + 1].Position.x;
				vz.y = m_Vertex[x][z - 1].Position.y - m_Vertex[x][z + 1].Position.y;
				vz.z = m_Vertex[x][z - 1].Position.z - m_Vertex[x][z + 1].Position.z;

				vn = Vector3::Cross(vz, vx);
				vn.Normalize();

				m_Vertex[x][z].Normal.x = vn.x;
				m_Vertex[x][z].Normal.y = vn.y;
				m_Vertex[x][z].Normal.z = vn.z;

			}
		}


		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(VERTEX_3D) * 21 * 21;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = m_Vertex;

		Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer);
	}

	//ファイルから高さ読み込み
	reLoad();


	// インデックスバッファ生成
	{
		unsigned int index[(22 * 2) * 20 - 2 ];

		int i = 0;
		for (int x = 0; x < 20; x++)
		{
			for (int z = 0; z < 21; z++)
			{
				index[i] = x * 21 + z;
				i++;

				index[i] = (x + 1) * 21 + z;
				i++;
			}

			if (x == 19)
				break;

			index[i] = (x + 1) * 21 + 20;
			i++;

			index[i] = (x + 1) * 21;
			i++;
		}

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(unsigned int) * ((22 * 2) * 20 - 2);
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = index;

		Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_IndexBuffer);
	}



	m_Texture = Texture::Load("asset\\texture\\field.jpg");
	m_TextureSand = Texture::Load("asset\\texture\\sand1k.png");



	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout,
		"shader\\vertexLightingVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader,
		"shader\\vertexLightingPS.cso");

}


void MeshField::Uninit()
{

	SaveY();

	m_VertexBuffer->Release();
	m_IndexBuffer->Release();
	//m_Texture->Release();

	m_PixelShader->Release();
	m_VertexLayout->Release();
	m_VertexShader->Release();

}


void MeshField::Update()
{
	if (Input::GetKeyPress('P'))
	{
		reLoad();
	}
}


void MeshField::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	// 頂点バッファ設定
	UINT stride = sizeof( VERTEX_3D );
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers( 0, 1, &m_VertexBuffer, &stride, &offset );

	// インデックスバッファ設定
	Renderer::GetDeviceContext()->IASetIndexBuffer( 
		m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0 );

	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	rot = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	trans = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	world = scale * rot * trans;
	Renderer::SetWorldMatrix(world);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory( &material, sizeof(material) );
	material.Diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	material.TextureEnable = true;
	Renderer::SetMaterial( material );

	// テクスチャ設定
	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);
	Renderer::GetDeviceContext()->PSSetShaderResources(1, 1, &m_TextureSand);

	// プリミティブトポロジ設定
	Renderer::GetDeviceContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	// ポリゴン描画
	Renderer::GetDeviceContext()->DrawIndexed(
		(22 * 2) * 20 - 2, 0, 0);

}



float MeshField::GetHeight(Vector3 Position)
{
	//これmeshfieldは複数のインスタンスで扱えた方がいいけどサイズの拡大は調点数を増やす方が無難だったりしそう
	int x, z;
	//ブロック番号算出
	x = Position.x / 5.0f + 10.0f;
	z = Position.z / -5.0f + 10.0f;

	XMFLOAT3 pos0, pos1, pos2, pos3;

	pos0 = m_Vertex[x][z].Position;
	pos1 = m_Vertex[x + 1][z].Position;
	pos2 = m_Vertex[x][z + 1].Position;
	pos3 = m_Vertex[x + 1][z + 1].Position;

	Vector3 v12, v1p;
	v12.x = pos2.x - pos1.x;
	v12.y = pos2.y - pos1.y;
	v12.z = pos2.z - pos1.z;

	v1p.x = Position.x - pos1.x;
	v1p.y = Position.y - pos1.y;
	v1p.z = Position.z - pos1.z;

	//外積
	float cy = v12.z * v1p.x - v12.x * v1p.z;

	float py;
	Vector3 n;
	if (cy > 0.0f)//符号で
	{
		//左上ポリゴン
		Vector3 v10;
		v10.x = pos0.x - pos1.x;
		v10.y = pos0.y - pos1.y;
		v10.z = pos0.z - pos1.z;

		//外積
		n = Vector3::Cross(v10, v12);
	}

	else
	{

		//右下ボリゴン
		Vector3 v13;
		v13.x = pos3.x - pos1.x;
		v13.y = pos3.y - pos1.y;
		v13.z = pos3.z - pos1.z;

		//外積
		n = Vector3::Cross(v12, v13);
	}

	//商さ取得
	py = -((Position.x - pos1.x) * n.x
		+ (Position.z - pos1.z) * n.z) 
		/ n.y + pos1.y;
	return py;
}



//ファイルに書き込み
void MeshField::SaveY()
{
	std::string output_csv_file_path = "test.csv";

	// 書き込むcsvファイルを開く(std::ofstreamのコンストラクタで開く)
	std::ofstream ofs_csv_file(output_csv_file_path);

	for (int z = 0; z < 20; z++)
	{
		for (int x = 0; x < 20; x++)
		{
			ofs_csv_file << m_Vertex[x][z].Position.y;

			if (x < 19) {
				ofs_csv_file << ',';
			}
		}
		ofs_csv_file << std::endl;
	}


}

//ファイルから値を設定
void MeshField::reLoad()
{
	std::string str_buf;
	std::string str_conma_buf;
	std::string input_csv_file_path = "test.csv";

	// 読み込むcsvファイルを開く(std::ifstreamのコンストラクタで開く)
	std::ifstream ifs_csv_file(input_csv_file_path);

	int z = 0;


	// getline関数で1行ずつ読み込む(読み込んだ内容はstr_bufに格納)
	while (getline(ifs_csv_file, str_buf)) {
		// 「,」区切りごとにデータを読み込むためにistringstream型にする
		std::istringstream i_stream(str_buf);
		int x = 0;
		// 「,」区切りごとにデータを読み込む
		while (getline(i_stream, str_conma_buf, ',')) {
		
			m_Vertex[x][z].Position.y = stof(str_conma_buf);
			x++;
		}
		z++;
		
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(VERTEX_3D) * 21 * 21;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.pSysMem = m_Vertex;

	Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer);

	// クローズ処理は不要[理由]ifstream型・ofstream型ともにデストラクタにてファイルクローズしてくれるため
}
