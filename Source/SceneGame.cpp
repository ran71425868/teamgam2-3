#include "System/Graphics.h"
#include "SceneGame.h"
#include "Camera.h"
#include "Player.h"
#include "EffectManager.h"

// 初期化
void SceneGame::Initialize()
{
	//ステージ初期化
	stage = new Stage();
	//プレイヤー初期化
	//player = new Player();
	Player::Instance().Initializa();

	//カメラ初期設定
	Graphics& graphics = Graphics::Instance();
	Camera& camera = Camera::Instance();
	camera.SetLookAt(
		DirectX::XMFLOAT3(0,10,-10),//視点
		DirectX::XMFLOAT3(0,0,0),//注視点
		DirectX::XMFLOAT3(0,1,0)//上方向
	);

	camera.SetPerspectiveFov(
		DirectX::XMConvertToRadians(45),//視野角
		graphics.GetScreenWidth() / graphics.GetScreenHeight(),//画面アスペクト比
		0.1f,//クリップ距離(近)
		1000.0f//クリップ距離(遠)
		);

	cameraController = new CameraController;

}

// 終了化
void SceneGame::Finalize()
{
	
	if (cameraController != nullptr) {
		delete cameraController;
		cameraController = nullptr;
	}

	//プレイヤー終了化
	/*if (player != nullptr)
	{
		delete player;
		player = nullptr;
	}*/
	Player::Instance().Finalize();

	//ステージ終了化
	if (stage != nullptr)
	{
		delete stage;
		stage = nullptr;
	}
}

// 更新処理
void SceneGame::Update(float elapsedTime)
{
	//カメラコントローラー更新処理
	DirectX::XMFLOAT3 target = Player::Instance().GetPosition();

	target.y += 0.5f;
	cameraController->SetTarget(target);
	cameraController->Update(elapsedTime);

	//ステージ更新処理
	stage->Update(elapsedTime);
	//プレイヤー更新処理
	Player::Instance().Update(elapsedTime);
	
	//エフェクトマネージャー更新処理
	EffectManager::Instance().Update(elapsedTime);
}

// 描画処理
void SceneGame::Render()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	ShapeRenderer* shapeRenderer = graphics.GetShapeRenderer();
	ModelRenderer* modelRenderer = graphics.GetModelRenderer();

	// 描画準備
	RenderContext rc;
	rc.deviceContext = dc;
	rc.lightDirection = { 0.0f, -1.0f, 0.0f };	// ライト方向（下方向）
	rc.renderState = graphics.GetRenderState();

	//カメラパラメータ設定
	Camera& camera = Camera::Instance();
	rc.view = camera.GetView();
	rc.projection = camera.GetProjection();


	// 3Dモデル描画
	{
		//ステージ描画
		stage->Render(rc, modelRenderer);
		//プレイヤー描画
		Player::Instance().Render(rc, modelRenderer);
		
		//エフェクトマネージャー描画
		EffectManager::Instance().Render(rc.view, rc.projection);
	}

	// 3Dデバッグ描画
	{
		//プレイヤーデバッグプリミティブ描画
		Player::Instance().RenderDebugPrimitive(rc, shapeRenderer);
		
	}

	// 2Dスプライト描画
	{

	}
}

// GUI描画
void SceneGame::DrawGUI()
{
	//プレイヤーデバッグ描画
	Player::Instance().DrawDebugGUI();
}
