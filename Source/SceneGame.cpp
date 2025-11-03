#include "System/Graphics.h"
#include <iostream>
#include "SceneGame.h"
#include "Camera.h"
#include "Player.h"
#include "Slime.h"
#include "EffectManager.h"

// 初期化
void SceneGame::Initialize()
{
	//ステージ初期化
	/*stage = new Stage();*/

	board = new Board();
	board->initialize();

	//プレイヤー初期化
	//player = new Player();
	Player::Instance().Initializa();

	// 駒を初期配置（例：白と黒のスライム）
	for (int i = 0; i <= 7; i++)
	{
		pieces.push_back(new Slime("white", { i, 1 }));//白　手前
		pieces.push_back(new Slime("black", { i, 6 }));//黒　奥
	}
	

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
		10000.0f//クリップ距離(遠)
		);

	cameraController = new CameraController;

	SetReady();

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

	if (board != nullptr)
	{
		delete board;
		board = nullptr;
	}

	for (auto p : pieces) delete p;
	pieces.clear();

}

// 更新処理
void SceneGame::Update(float elapsedTime)
{
	Mouse& mouseCursor = Input::Instance().GetMouse();
	//mouseCursor.Update(); // 毎フレーム更新

	if (mouseCursor.GetButtonDown() & Mouse::BTN_LEFT) {
		POINT cursor = mouseCursor.GetPosition();
		Position clicked = ScreenToBoard(cursor.x, cursor.y);

		if (!board->isInsideBoard(clicked)) return;

		auto clickedPiece = board->getPieceAt(clicked);

		if (selectedPos.x == -1) {
			// 駒を選択
			if (clickedPiece && clickedPiece->getColor() == board->getCurrentTurn()) {
				if (clickedPiece->getColor() == "black") return;
				selectedPos = clicked;
				legalMoves = clickedPiece->getLegalMoves(*board);
			}
		}
		else {
			// 移動先として合法か判定
			for (auto& move : legalMoves) {
				if (move.x == clicked.x && move.y == clicked.y) {
					board->movePiece(selectedPos, clicked);

					auto slime = FindSlimeAt(selectedPos);
					if (slime) slime->SetBoardPosition(clicked);

					selectedPos = { -1, -1 };
					legalMoves.clear();
					board->switchTurn();
					return;
				}
			}

			// 不正な場所 → 選択解除
			selectedPos = { -1, -1 };
			legalMoves.clear();
		}
	}


	stage->Update(elapsedTime);
	for (auto p : pieces) p->Update(elapsedTime);


	//カメラコントローラー更新処理

	DirectX::XMFLOAT3 Poswhite{ 400.0f,0.0f,300.0f };
	DirectX::XMFLOAT3 Posblack{ 400.0f,0.0f,600.0f };


	DirectX::XMFLOAT3 target = Player::Instance().GetPosition();

	//DirectX::XMFLOAT3 target;
	target.x = Poswhite.x;
	target.y = 0.0f;
	target.z = Posblack.z - Poswhite.z;

	target.y += 0.5f;
	cameraController->SetTarget(target);
	cameraController->Update(elapsedTime);

	//ステージ更新処理
	//stage->Update(elapsedTime);

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

	ModelRenderer* renderer = graphics.GetModelRenderer(); // ←環境に応じて取得方法を調整
	stage->Render(rc, renderer);
	for (auto p : pieces) p->Render(rc, renderer);


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

		/*slime->Render(rc, modelRenderer);*/

		for (auto p : pieces) {
			p->Render(rc, renderer);
		}
		
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

//マウス座標 → 盤面座標変換
Position SceneGame::ScreenToBoard(int screenX, int screenY) {
	int boardX = screenX / 100;
	int boardY = screenY / 100;
	return { boardX, boardY };
}

//Slime を盤面座標から取得
Slime* SceneGame::FindSlimeAt(Position pos) {
	for (auto slime : pieces) {
		if (slime->GetBoardPosition().x == pos.x &&
			slime->GetBoardPosition().y == pos.y)
			return slime;
	}
	return nullptr;
}
