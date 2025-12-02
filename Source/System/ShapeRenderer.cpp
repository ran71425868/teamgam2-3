#include "Misc.h"
#include "GpuResourceUtils.h"
#include "ShapeRenderer.h"
#include "Graphics.h"

// コンストラクタ
ShapeRenderer::ShapeRenderer(ID3D11Device* device)
{
	// 入力レイアウト
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// 頂点シェーダー
	GpuResourceUtils::LoadVertexShader(
		device,
		"Data/Shader/ShapeRendererVS.cso",
		inputElementDesc,
		_countof(inputElementDesc),
		inputLayout.GetAddressOf(),
		vertexShader.GetAddressOf());

	// ピクセルシェーダー
	GpuResourceUtils::LoadPixelShader(
		device,
		"Data/Shader/ShapeRendererPS.cso",
		pixelShader.GetAddressOf());

	// 定数バッファ
	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbMesh),
		constantBuffer.GetAddressOf());

	// 箱メッシュ生成
	CreateBoxMesh(device, 1.0f, 1.0f, 1.0f);

	// 球メッシュ生成
	CreateSphereMesh(device, 1.0f, 32);

	// 半球メッシュ生成
	CreateHalfSphereMesh(device, 1.0f, 32);

	// 円柱メッシュ生成
	CreateCylinderMesh(device, 1.0f, 1.0f, -0.5f, 1.0f, 32);

	// 2D矩形メッシュ生成 (幅1, 高さ1, 奥行きは無視)
	CreateRectMesh(device, 1.0f, 1.0f);
}

// 箱描画
void ShapeRenderer::RenderBox(
	const RenderContext& rc,
	const DirectX::XMFLOAT3& position,
	const DirectX::XMFLOAT3& angle,
	const DirectX::XMFLOAT3& size,
	const DirectX::XMFLOAT4& color) const
{
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(size.x, size.y, size.z);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	DirectX::XMFLOAT4X4 transform;
	DirectX::XMStoreFloat4x4(&transform, S * R * T);

	Render(rc, boxMesh, transform, color);
}

// 球描画
void ShapeRenderer::RenderSphere(
	const RenderContext& rc,
	const DirectX::XMFLOAT3& position,
	float radius,
	const DirectX::XMFLOAT4& color) const
{
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(radius, radius, radius);
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	DirectX::XMFLOAT4X4 transform;
	DirectX::XMStoreFloat4x4(&transform, S * T);

	Render(rc, sphereMesh, transform, color);
}

// 円柱描画
void ShapeRenderer::RenderCylinder(
	const RenderContext& rc,
	const DirectX::XMFLOAT3& position,
	float radius,
	float height,
	const DirectX::XMFLOAT4& color) const
{
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(radius, height, radius);
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y + height * 0.5f, position.z);
	DirectX::XMFLOAT4X4 transform;
	DirectX::XMStoreFloat4x4(&transform, S * T);

	Render(rc, cylinderMesh, transform, color);
}

// カプセル描画
void ShapeRenderer::RenderCapsule(
	const RenderContext& rc,
	const DirectX::XMFLOAT4X4& transform,
	float radius,
	float height,
	const DirectX::XMFLOAT4& color) const
{
	DirectX::XMMATRIX Transform = DirectX::XMLoadFloat4x4(&transform);

	// 上半球
	{
		DirectX::XMVECTOR Position = DirectX::XMVector3Transform(DirectX::XMVectorSet(0, height * 0.5f, 0, 0), Transform);
		DirectX::XMMATRIX World = DirectX::XMMatrixScaling(radius, radius, radius);
		World.r[3] = DirectX::XMVectorSetW(Position, 1.0f);
		DirectX::XMFLOAT4X4 world;
		DirectX::XMStoreFloat4x4(&world, World);
		Render(rc, halfSphereMesh, world, color);
	}
	// 円柱
	{
		DirectX::XMMATRIX World;
		World.r[0] = DirectX::XMVectorScale(Transform.r[0], radius);
		World.r[1] = DirectX::XMVectorScale(Transform.r[1], height);
		World.r[2] = DirectX::XMVectorScale(Transform.r[2], radius);
		World.r[3] = Transform.r[3];
		DirectX::XMFLOAT4X4 world;
		DirectX::XMStoreFloat4x4(&world, World);
		Render(rc, cylinderMesh, world, color);
	}
	// 下半球
	{
		DirectX::XMMATRIX World = DirectX::XMMatrixRotationX(DirectX::XM_PI);
		DirectX::XMVECTOR Position = DirectX::XMVector3Transform(DirectX::XMVectorSet(0, -height * 0.5f, 0, 0), Transform);
		Transform.r[3] = DirectX::XMVectorSet(0, 0, 0, 1);
		World = DirectX::XMMatrixMultiply(World, Transform);
		World.r[0] = DirectX::XMVectorScale(World.r[0], radius);
		World.r[1] = DirectX::XMVectorScale(World.r[1], radius);
		World.r[2] = DirectX::XMVectorScale(World.r[2], radius);
		World.r[3] = DirectX::XMVectorSetW(Position, 1.0f);
		DirectX::XMFLOAT4X4 world;
		DirectX::XMStoreFloat4x4(&world, World);
		Render(rc, halfSphereMesh, world, color);
	}
}

// メッシュ生成
void ShapeRenderer::CreateMesh(ID3D11Device* device, const std::vector<DirectX::XMFLOAT3>& vertices, Mesh& mesh)
{
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = static_cast<UINT>(sizeof(DirectX::XMFLOAT3) * vertices.size());
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pSysMem = vertices.data();
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&desc, &subresourceData, mesh.vertexBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	mesh.vertexCount = static_cast<UINT>(vertices.size());
}

// 箱メッシュ作成
void ShapeRenderer::CreateBoxMesh(ID3D11Device* device, float width, float height, float depth)
{
	DirectX::XMFLOAT3 positions[8] =
	{
		// top
		{ -width,  height, -depth},
		{  width,  height, -depth},
		{  width,  height,  depth},
		{ -width,  height,  depth},
		// bottom
		{ -width, -height, -depth},
		{  width, -height, -depth},
		{  width, -height,  depth},
		{ -width, -height,  depth},
	};

	std::vector<DirectX::XMFLOAT3> vertices;
	vertices.resize(32);

	// top
	vertices.emplace_back(positions[0]);
	vertices.emplace_back(positions[1]);
	vertices.emplace_back(positions[1]);
	vertices.emplace_back(positions[2]);
	vertices.emplace_back(positions[2]);
	vertices.emplace_back(positions[3]);
	vertices.emplace_back(positions[3]);
	vertices.emplace_back(positions[0]);
	// bottom
	vertices.emplace_back(positions[4]);
	vertices.emplace_back(positions[5]);
	vertices.emplace_back(positions[5]);
	vertices.emplace_back(positions[6]);
	vertices.emplace_back(positions[6]);
	vertices.emplace_back(positions[7]);
	vertices.emplace_back(positions[7]);
	vertices.emplace_back(positions[4]);
	// side
	vertices.emplace_back(positions[0]);
	vertices.emplace_back(positions[4]);
	vertices.emplace_back(positions[1]);
	vertices.emplace_back(positions[5]);
	vertices.emplace_back(positions[2]);
	vertices.emplace_back(positions[6]);
	vertices.emplace_back(positions[3]);
	vertices.emplace_back(positions[7]);

	// メッシュ生成
	CreateMesh(device, vertices, boxMesh);
}

// 球メッシュ作成
void ShapeRenderer::CreateSphereMesh(ID3D11Device* device, float radius, int subdivisions)
{
	float step = DirectX::XM_2PI / subdivisions;

	std::vector<DirectX::XMFLOAT3> vertices;

	// XZ平面
	for (int i = 0; i < subdivisions; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			float theta = step * ((i + j) % subdivisions);

			DirectX::XMFLOAT3& p = vertices.emplace_back();
			p.x = sinf(theta) * radius;
			p.y = 0.0f;
			p.z = cosf(theta) * radius;
		}
	}
	// XY平面
	for (int i = 0; i < subdivisions; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			float theta = step * ((i + j) % subdivisions);

			DirectX::XMFLOAT3& p = vertices.emplace_back();
			p.x = sinf(theta) * radius;
			p.y = cosf(theta) * radius;
			p.z = 0.0f;
		}
	}
	// YZ平面
	for (int i = 0; i < subdivisions; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			float theta = step * ((i + j) % subdivisions);

			DirectX::XMFLOAT3& p = vertices.emplace_back();
			p.x = 0.0f;
			p.y = sinf(theta) * radius;
			p.z = cosf(theta) * radius;
		}
	}

	// メッシュ生成
	CreateMesh(device, vertices, sphereMesh);
}

// 半球メッシュ作成
void ShapeRenderer::CreateHalfSphereMesh(ID3D11Device* device, float radius, int subdivisions)
{
	std::vector<DirectX::XMFLOAT3> vertices;

	float theta_step = DirectX::XM_2PI / subdivisions;

	// XZ平面
	for (int i = 0; i < subdivisions; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			float theta = theta_step * ((i + j) % subdivisions);

			DirectX::XMFLOAT3& v = vertices.emplace_back();

			v.x = sinf(theta) * radius;
			v.y = 0.0f;
			v.z = cosf(theta) * radius;
		}
	}
	// XY平面
	for (int i = 0; i < subdivisions / 2; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			float theta = theta_step * ((i + j) % subdivisions) - DirectX::XM_PIDIV2;

			DirectX::XMFLOAT3& v = vertices.emplace_back();

			v.x = sinf(theta) * radius;
			v.y = cosf(theta) * radius;
			v.z = 0.0f;
		}
	}
	// YZ平面
	for (int i = 0; i < subdivisions / 2; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			float theta = theta_step * ((i + j) % subdivisions);

			DirectX::XMFLOAT3& v = vertices.emplace_back();

			v.x = 0.0f;
			v.y = sinf(theta) * radius;
			v.z = cosf(theta) * radius;
		}
	}

	// メッシュ生成
	CreateMesh(device, vertices, halfSphereMesh);
}

// 円柱
void ShapeRenderer::CreateCylinderMesh(ID3D11Device* device, float radius1, float radius2, float start, float height, int subdivisions)
{
	std::vector<DirectX::XMFLOAT3> vertices;

	float theta_step = DirectX::XM_2PI / subdivisions;

	// XZ平面
	for (int i = 0; i < subdivisions; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			float theta = theta_step * ((i + j) % subdivisions);

			DirectX::XMFLOAT3& v = vertices.emplace_back();

			v.x = sinf(theta) * radius1;
			v.y = start;
			v.z = cosf(theta) * radius1;
		}
	}
	for (int i = 0; i < subdivisions; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			float theta = theta_step * ((i + j) % subdivisions);

			DirectX::XMFLOAT3& v = vertices.emplace_back();

			v.x = sinf(theta) * radius2;
			v.y = start + height;
			v.z = cosf(theta) * radius2;
		}
	}
	// XY平面
	{
		vertices.emplace_back(DirectX::XMFLOAT3(0.0f, start, radius1));
		vertices.emplace_back(DirectX::XMFLOAT3(0.0f, start + height, radius2));
		vertices.emplace_back(DirectX::XMFLOAT3(0.0f, start, -radius1));
		vertices.emplace_back(DirectX::XMFLOAT3(0.0f, start + height, -radius2));
	}
	// YZ平面
	{
		vertices.emplace_back(DirectX::XMFLOAT3(radius1, start, 0.0f));
		vertices.emplace_back(DirectX::XMFLOAT3(radius2, start + height, 0.0f));
		vertices.emplace_back(DirectX::XMFLOAT3(-radius1, start, 0.0f));
		vertices.emplace_back(DirectX::XMFLOAT3(-radius2, start + height, 0.0f));
	}

	// メッシュ生成
	CreateMesh(device, vertices, cylinderMesh);
}

// ShapeRenderer::DrawRect
void ShapeRenderer::DrawRect(
	const RenderContext& rc,
	float x, float y, float w, float h,
	const DirectX::XMFLOAT4& color) const
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	// 1. 頂点データの設定: プリミティブトポロジーをTRIANGLELISTに変更
	// 既存の Render メソッドが LINELIST のままなので、ここで上書きします。
	// ※ DrawRect 専用の Render メソッドを作成するか、Renderメソッドを拡張するのが理想ですが、
	// ここでは DrawRect の中で直接トポロジーを設定します。
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 2. WVP行列 (ワールドビュープロジェクション行列) の計算

	// 描画を2Dスクリーン座標系で行うため、WVP行列を NDC (Normalized Device Coordinates) への変換に設定します。
	// NDCは X/Y が [-1, 1] の範囲です。

	// スクリーンサイズを取得 (Graphics::Instance() から取得)
	float screenWidth = Graphics::Instance().GetScreenWidth();
	float screenHeight = Graphics::Instance().GetScreenHeight();

	// 矩形の中点位置 (スクリーン中心を (0,0) とするNDC基準)
	// X, Y座標を [0, ScreenSize] から [-1, 1] (NDC) に変換
	float centerX = (x + w * 0.5f);
	float centerY = (y + h * 0.5f);

	float ndcX = (2.0f * centerX / screenWidth) - 1.0f;
	float ndcY = 1.0f - (2.0f * centerY / screenHeight); // Y軸反転

	// NDCでの幅と高さ (NDCの全幅/全高は 2.0)
	float ndcWidth = w * 2.0f / screenWidth;
	float ndcHeight = h * 2.0f / screenHeight;

	// NDCスケール・平行移動行列の作成
	// Scale行列でNDCの幅/高さを設定し、Translate行列でNDCの位置に移動
	DirectX::XMMATRIX WVP =
		DirectX::XMMatrixScaling(ndcWidth, ndcHeight, 1.0f) * DirectX::XMMatrixTranslation(ndcX, ndcY, 0.0f);

	// 3. 描画処理 (Renderメソッドの処理をコピーして修正)

	// シェーダー設定
	dc->VSSetShader(vertexShader.Get(), nullptr, 0);
	dc->PSSetShader(pixelShader.Get(), nullptr, 0);
	dc->IASetInputLayout(inputLayout.Get());

	// 定数バッファ設定
	dc->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

	// 頂点バッファ設定
	UINT stride = sizeof(DirectX::XMFLOAT3);
	UINT offset = 0;
	dc->IASetVertexBuffers(0, 1, rectMesh.vertexBuffer.GetAddressOf(), &stride, &offset);

	// 定数バッファ更新
	CbMesh cbMesh;
	DirectX::XMStoreFloat4x4(&cbMesh.worldViewProjection, WVP);
	cbMesh.color = color;

	dc->UpdateSubresource(constantBuffer.Get(), 0, 0, &cbMesh, 0, 0);

	// 描画
	dc->Draw(rectMesh.vertexCount, 0);

	// 🚨 注意: 描画後、トポロジーを元に戻す必要がある
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

void ShapeRenderer::DrawRectBorder(
	const RenderContext& rc,
	float dx, float dy, 
	float dw, float dh, 
	DirectX::XMFLOAT4 color, 
	float thickness)
{
	// 枠線を描画するため、4つの細い矩形(DrawRect) を使用する。

		// 1. 上側の線
	DrawRect(
		rc,                 // ★RenderContextを渡す
		dx,                 // X座標 (左上)
		dy,                 // Y座標 (左上)
		dw,                 // 幅
		thickness,          // 高さ
		color
	);

	// 2. 下側の線
	DrawRect(
		rc,                 // ★RenderContextを渡す
		dx,                 // X座標 (左上)
		dy + dh - thickness, // Y座標 (下端から太さ分上に移動)
		dw,                 // 幅
		thickness,          // 高さ
		color
	);

	// 3. 左側の線
	DrawRect(
		rc,                 // ★RenderContextを渡す
		dx,                 // X座標 (左上)
		dy + thickness,     // Y座標 (上側の線の下端から開始)
		thickness,          // 幅 (太さ)
		dh - thickness * 2, // 高さ (上下の線が占有する部分を除外)
		color
	);

	// 4. 右側の線
	DrawRect(
		rc,                 // ★RenderContextを渡す
		dx + dw - thickness, // X座標 (右端から太さ分左に移動)
		dy + thickness,     // Y座標 (上側の線の下端から開始)
		thickness,          // 幅 (太さ)
		dh - thickness * 2, // 高さ (上下の線が占有する部分を除外)
		color
	);
}

// 描画実行
void ShapeRenderer::Render(const RenderContext& rc, const Mesh& mesh, const DirectX::XMFLOAT4X4& transform, const DirectX::XMFLOAT4& color) const
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	// シェーダー設定
	dc->VSSetShader(vertexShader.Get(), nullptr, 0);
	dc->PSSetShader(pixelShader.Get(), nullptr, 0);
	dc->IASetInputLayout(inputLayout.Get());

	// 定数バッファ設定
	dc->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

	// ビュープロジェクション行列作成
	DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&rc.view);
	DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&rc.projection);
	DirectX::XMMATRIX VP = V * P;

	// プリミティブ設定
	UINT stride = sizeof(DirectX::XMFLOAT3);
	UINT offset = 0;
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// 頂点バッファ設定
	dc->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);

	// ワールドビュープロジェクション行列作成
	DirectX::XMMATRIX W = DirectX::XMLoadFloat4x4(&transform);
	DirectX::XMMATRIX WVP = W * VP;

	// 定数バッファ更新
	CbMesh cbMesh;
	DirectX::XMStoreFloat4x4(&cbMesh.worldViewProjection, WVP);
	cbMesh.color = color;

	dc->UpdateSubresource(constantBuffer.Get(), 0, 0, &cbMesh, 0, 0);

	// 描画
	dc->Draw(mesh.vertexCount, 0);
}

// ShapeRenderer::CreateRectMesh
// 矩形を構成する2つの三角形（塗りつぶし用）を生成
void ShapeRenderer::CreateRectMesh(ID3D11Device* device, float width, float height)
{
	std::vector<DirectX::XMFLOAT3> vertices;

	// 中心を (0, 0) とし、幅 width, 高さ height の矩形を生成
	float w = width * 0.5f;
	float h = height * 0.5f;

	// 頂点データ (時計回り: D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST 用)
	// 矩形の角:
	// V0: (-w, +h, 0) (左上)
	// V1: (+w, +h, 0) (右上)
	// V2: (+w, -h, 0) (右下)
	// V3: (-w, -h, 0) (左下)

	// 1つ目の三角形 (左上, 右上, 右下)
	vertices.emplace_back(DirectX::XMFLOAT3(-w, +h, 0.0f)); // V0
	vertices.emplace_back(DirectX::XMFLOAT3(+w, +h, 0.0f)); // V1
	vertices.emplace_back(DirectX::XMFLOAT3(+w, -h, 0.0f)); // V2

	// 2つ目の三角形 (左上, 右下, 左下)
	vertices.emplace_back(DirectX::XMFLOAT3(-w, +h, 0.0f)); // V0
	vertices.emplace_back(DirectX::XMFLOAT3(+w, -h, 0.0f)); // V2
	vertices.emplace_back(DirectX::XMFLOAT3(-w, -h, 0.0f)); // V3

	// メッシュ生成 (CreateMeshは既存のラッパーを使用)
	CreateMesh(device, vertices, rectMesh);
}