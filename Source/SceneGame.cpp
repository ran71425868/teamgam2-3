#include "System/Graphics.h"
#include <iostream>
#include "SceneGame.h"
#include "Camera.h"
#include "Player.h"
#include "Slime.h"
#include "EffectManager.h"
#include <DirectXMath.h>
using namespace DirectX;

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
	
	//king
	//pieces.push_back(new Slime("white", { 4, 0 }));
	////rook
	//pieces.push_back(new Slime("white", { 0, 0 }));
	//pieces.push_back(new Slime("white", { 7, 0 }));
	////knight
	//pieces.push_back(new Slime("white", { 1, 0 }));
	//pieces.push_back(new Slime("white", { 6, 0 }));
	////bishop
	//pieces.push_back(new Slime("white", { 2, 0 }));
	//pieces.push_back(new Slime("white", { 5, 0 }));
	////queen
	//pieces.push_back(new Slime("white", { 3, 0 }));


	//king
	//pieces.push_back(new Slime("black", { 4, 7 }));
	////rook
	//pieces.push_back(new Slime("black", { 0, 7 }));
	//pieces.push_back(new Slime("black", { 7, 7 }));
	////knight
	//pieces.push_back(new Slime("black", { 1, 7 }));
	//pieces.push_back(new Slime("black", { 6, 7 }));
	////bishop
	//pieces.push_back(new Slime("black", { 2, 7 }));
	//pieces.push_back(new Slime("black", { 5, 7 }));
	////queen
	//pieces.push_back(new Slime("black", { 3, 7 }));
	isServer = true; // ← サーバー側なら true / クライアントなら false に設定
	if (isServer)
		network.Initialize(NetworkManager::Mode::Server, "0.0.0.0", 50000);
	else
		network.Initialize(NetworkManager::Mode::Client, "192.168.0.2", 50000);


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

	network.Finalize();

}

// 更新処理
void SceneGame::Update(float elapsedTime)
{
	Mouse& mouseCursor = Input::Instance().GetMouse();

	if (mouseCursor.GetButtonDown() & Mouse::BTN_LEFT) {
		POINT cursor = mouseCursor.GetPosition();

		// 1. ボード座標への変換とボード外チェック
		Position clicked = ScreenToBoard(cursor.x, cursor.y);
		if (!board->isInsideBoard(clicked)) return; // ボード外なら何もしない

		auto clickedPiece = board->getPieceAt(clicked);

		// --- 2. 駒が選択されていない場合 (selectedPos.x == -1) ---
		if (selectedPos.x == -1) {
			// 自分の駒をクリックした場合、それを選択する
			if (clickedPiece) {
				auto color = clickedPiece->getColor();
				auto currentTurn = board->getCurrentTurn();

				// クリックした駒が現在のターンプレイヤーの駒であれば選択
				if (color == currentTurn) {
					selectedPos = clicked;
					legalMoves = clickedPiece->getLegalMoves(*board);
					// この時点で処理を終了し、選択状態を維持
					return;
				}
			}
		}
		// --- 3. 駒が既に選択されている場合 (else) ---
		else {
			// 移動先として合法か判定
			bool isLegalMove = false;
			for (auto& move : legalMoves) {
				if (move.x == clicked.x && move.y == clicked.y) {
					isLegalMove = true;
					break; // 合法な移動先を見つけたのでループを抜ける
				}
			}

			
			if (isLegalMove) {
				// 🟢 合法な移動を実行
				board->movePiece(selectedPos, clicked);

				// slime の位置も更新
				auto slime = FindSlimeAt(selectedPos);
				if (slime) slime->SetBoardPosition(clicked);

				MoveData move{ 1, selectedPos.x, selectedPos.y, clicked.x, clicked.y };
				network.SendMove(move);


				// 選択を解除し、ターンを切り替える
				selectedPos = { -1, -1 };
				legalMoves.clear();
				board->switchTurn();
				return;
			}
			else {
				// 🔴 不正な場所をクリックした場合

				// パターンA: 別の自分の駒をクリック → 選択を切り替える
				// パターンB: 何もないマスや敵の駒をクリック → 選択解除

				// 別の自分の駒をクリックしたかチェックし、選択を切り替えるロジックをここに追加できます。

				// シンプルに選択解除のみを行う
				selectedPos = { -1, -1 };
				legalMoves.clear();

			}
		}
	}
	//Mouse& mouseCursor = Input::Instance().GetMouse();
	////mouseCursor.Update(); // 毎フレーム更新
	//
	//if (mouseCursor.GetButtonDown() & Mouse::BTN_LEFT) {
	//	POINT cursor = mouseCursor.GetPosition();
	//
	//	// 今はざつい
	//	Position clicked = ScreenToBoard(cursor.x, cursor.y);
	//	if (!board->isInsideBoard(clicked)) return;
	//
	//	auto clickedPiece = board->getPieceAt(clicked);
	//
	//	if (selectedPos.x == -1) {
	//		// 駒を選択
	//		if (clickedPiece) 
	//		{
	//			auto color = clickedPiece->getColor();
	//			auto currentTurn = board->getCurrentTurn();
	//
	//			if ( color == currentTurn ) {
	//				/*if (clickedPiece->getColor() == "black") return;*/
	//				selectedPos = clicked;
	//				legalMoves = clickedPiece->getLegalMoves(*board);
	//
	//				// 移動先として合法か判定
	//				/*board->movePiece(selectedPos, clicked);
	//
	//				auto slime = FindSlimeAt(selectedPos);
	//
	//				if (slime) slime->SetBoardPosition(clicked);
	//
	//				selectedPos = { -1, -1 };
	//				legalMoves.clear();
	//				board->switchTurn();
	//				return;*/
	//			}
	//		}
	//	}
	//	else {
	//		// 移動先として合法か判定
	//		for (auto& move : legalMoves) {
	//			if (move.x == clicked.x && move.y == clicked.y) {
	//				board->movePiece(selectedPos, clicked);
	//				// slime の位置も更新
	//				auto slime = FindSlimeAt(selectedPos);
	//				if (slime) slime->SetBoardPosition(clicked);
	//
	//				selectedPos = { -1, -1 };
	//				legalMoves.clear();
	//				board->switchTurn();
	//				return;
	//			}
	//		}
	//		// 不正な場所 → 選択解除
	//		selectedPos = { -1, -1 };
	//		legalMoves.clear();
	//	}
	//}

	MoveData recvMove{};
	if (network.ReceiveMove(recvMove)) {
		Position from{ recvMove.fromX, recvMove.fromY };
		Position to{ recvMove.toX, recvMove.toY };
		board->movePiece(from, to);
		auto slime = FindSlimeAt(from);
		if (slime) slime->SetBoardPosition(to);
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
Position SceneGame::ScreenToBoard(int screenX, int screenY)
{
	using namespace DirectX;

	// スクリーンサイズ
	int screenWidth = Graphics::Instance().GetScreenWidth();
	int screenHeight = Graphics::Instance().GetScreenHeight();

	// スクリーン座標 → NDC（-1〜+1）
	float ndcX = (2.0f * screenX / screenWidth) - 1.0f;
	float ndcY = 1.0f - (2.0f * screenY / screenHeight); // Y反転

	// ビュー・プロジェクション行列
	XMMATRIX view = XMLoadFloat4x4(&Camera::Instance().GetView());
	XMMATRIX proj = XMLoadFloat4x4(&Camera::Instance().GetProjection());
	XMMATRIX invViewProj = XMMatrixInverse(nullptr, view * proj);

	// NDC → ワールド空間レイ
	XMVECTOR nearPoint = XMVectorSet(ndcX, ndcY, 0.0f, 1.0f);
	XMVECTOR farPoint = XMVectorSet(ndcX, ndcY, 1.0f, 1.0f);

	nearPoint = XMVector3TransformCoord(nearPoint, invViewProj);
	farPoint = XMVector3TransformCoord(farPoint, invViewProj);

	XMVECTOR rayOrigin = nearPoint;
	XMVECTOR rayDir = XMVector3Normalize(farPoint - nearPoint);

	// レイと盤面（Y=0）との交点を求める
	float rayY = XMVectorGetY(rayDir);
	if (fabs(rayY) < 1e-5f) return { -1, -1 };

	float t = -XMVectorGetY(rayOrigin) / rayY;
	if (t < 0) return { -1, -1 };

	XMVECTOR hitPos = rayOrigin + rayDir * t;

	float x = XMVectorGetX(hitPos);
	float z = XMVectorGetZ(hitPos);

	// ✅ 原点補正（盤面の描画開始位置）
	constexpr float boardOriginX = -50.0f; // ← 必要に応じて -50.0f などに調整
	constexpr float boardOriginZ = 0.0f;

	float localX = x - boardOriginX;
	float localZ = z - boardOriginZ;

	// ✅ 範囲チェック（盤面サイズ 8x8, 1マス100）
	if (localX < 0 || localX >= 800 || localZ < 0 || localZ >= 800)
		return { -1, -1 };

	// ✅ floorで丸め誤差を防止
	int boardX = static_cast<int>(std::floor(localX / 100.0f));
	int boardY = static_cast<int>(std::floor(localZ / 100.0f));

	return { boardX, boardY };
}
//Position SceneGame::ScreenToBoard(int screenX, int screenY)
//{
//	// スクリーンサイズ
//	int screenWidth = Graphics::Instance().GetScreenWidth();
//	int screenHeight = Graphics::Instance().GetScreenHeight();
//
//	// スクリーン座標 → NDC（-1〜+1）
//	float ndcX = (2.0f * screenX / screenWidth) - 1.0f;
//	float ndcY = 1.0f - (2.0f * screenY / screenHeight); // Y反転
//
//	// ビュー・プロジェクション行列
//	XMMATRIX view = XMLoadFloat4x4(&Camera::Instance().GetView());
//	XMMATRIX proj = XMLoadFloat4x4(&Camera::Instance().GetProjection());
//	XMMATRIX invViewProj = XMMatrixInverse(nullptr, view * proj);
//
//	// NDC → ワールド座標（near/far）
//	XMVECTOR nearPoint = XMVectorSet(ndcX, ndcY, 0.0f, 1.0f);
//	XMVECTOR farPoint = XMVectorSet(ndcX, ndcY, 1.0f, 1.0f);
//
//	nearPoint = XMVector3TransformCoord(nearPoint, invViewProj);
//	farPoint = XMVector3TransformCoord(farPoint, invViewProj);
//
//	// レイの原点と方向
//	XMVECTOR rayOrigin = nearPoint;
//	XMVECTOR rayDir = XMVector3Normalize(farPoint - nearPoint);
//
//	// レイと盤面（Y=0）との交差判定
//	float rayY = XMVectorGetY(rayDir);
//	if (fabs(rayY) < 1e-5f) return { -1, -1 }; // 水平レイは無効
//
//	float t = -XMVectorGetY(rayOrigin) / rayY;
//	if (t < 0) return { -1, -1 }; // 盤面の下に向かっていない
//
//	XMVECTOR hitPos = rayOrigin + rayDir * t;
//
//	float x = XMVectorGetX(hitPos);
//	float z = XMVectorGetZ(hitPos);
//
//	// 盤面の範囲チェック（0〜800）
//	float boardOriginX = 0.0f;
//	float boardOriginZ = 0.0f;
//
//	float localX = x - boardOriginX;
//	float localZ = z - boardOriginZ;
//
//	if (localX < 0 || localX >= 800 || localZ < 0 || localZ >= 800)
//		return { -1, -1 };
//
//	int boardX = static_cast<int>(std::floor(localX / 100.0f));
//	int boardY = static_cast<int>(std::floor(localZ / 100.0f));
//
//
//	// マス座標に変換（1マス = 100）
//	/*int boardX = static_cast<int>(std::floor(x / 100.0f));
//	int boardY = static_cast<int>(std::floor(z / 100.0f));*/
//
//	return { boardX, boardY };
//}

//Slime を盤面座標から取得
Slime* SceneGame::FindSlimeAt(Position pos) {
	for (auto slime : pieces) {
		if (slime->GetBoardPosition().x == pos.x &&
			slime->GetBoardPosition().y == pos.y)
			return slime;
	}
	return nullptr;
}
