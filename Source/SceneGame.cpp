#include <random>
#include <algorithm> // std::shuffle用
#include "System/Graphics.h"
#include <iostream>
#include "SceneGame.h"
#include "SceneWhiteResult.h"
#include"SceneBlackResult.h"
#include "SceneManager.h"
#include "Camera.h"
#include "Piece.h"
#include "EffectManager.h"
#include <DirectXMath.h>


using namespace DirectX;


// 初期化
void SceneGame::Initialize()
{

	//ステージ初期化
	board = new Board();
	board->initialize();

	highlightModel = new Model("Data/Model/Stage/yellow_bord.mdl");
	healSpotModel = new Model("Data/Model/Stage/heal_bord.mdl");


	// 駒を初期配置（例：白と黒のスライム）
	for (int i = 0; i <= 7; i++)
	{
		pieces.push_back(new Piece("white", { i, 1 },"pawn"));//白　手前
		pieces.push_back(new Piece("black", { i, 6 },"pawn"));//黒　奥
	}
	
	//king
	pieces.push_back(new Piece("white", { 4, 0 },"king"));
	//rook
	pieces.push_back(new Piece("white", { 0, 0 },"rook"));
	pieces.push_back(new Piece("white", { 7, 0 },"rook"));
	//knight
	pieces.push_back(new Piece("white", { 1, 0 },"knight"));
	pieces.push_back(new Piece("white", { 6, 0 },"knight"));
	//bishop
	pieces.push_back(new Piece("white", { 2, 0 },"bishop"));
	pieces.push_back(new Piece("white", { 5, 0 },"bishop"));
	//queen
	pieces.push_back(new Piece("white", { 3, 0 },"queen"));


	//king
	pieces.push_back(new Piece("black", { 4, 7 },"king"));
	//rook
	pieces.push_back(new Piece("black", { 0, 7 },"rook"));
	pieces.push_back(new Piece("black", { 7, 7 },"rook"));
	//knight
	pieces.push_back(new Piece("black", { 1, 7 },"knight"));
	pieces.push_back(new Piece("black", { 6, 7 },"knight"));
	//bishop
	pieces.push_back(new Piece("black", { 2, 7 },"bishop"));
	pieces.push_back(new Piece("black", { 5, 7 },"bishop"));
	//queen
	pieces.push_back(new Piece("black", { 3, 7 },"queen"));

	
	//isServer = true; // ← サーバー側なら true / クライアントなら false に設定
	//if (isServer)
	//	network.Initialize(NetworkManager::Mode::Server, "0.0.0.0", 50000);
	//else
		/*network.Initialize(NetworkManager::Mode::Client, "192.168.0.2", 50000);*/

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

	ai = new ChessAI();
	ai->onMoveCallback = [this](Position from, Position to, bool wasCapture) {
		// 1) 敵駒が取られた場合の処理 (toの位置の描画オブジェクトを削除)
		if (wasCapture) {
			this->RemovePieceAt(to);
		}

		// 2) 移動元の描画オブジェクトを取得
		auto piece = this->FindSlimeAt(from);

		if (piece) {
			// 3) from と to が同じなら、移動ではなく「fromでの死亡」を意味する
			if (from.x == to.x && from.y == to.y) {
				// AIの自傷死亡または戦闘敗北による移動元での死亡
				this->RemovePieceAt(from);
			}
			else {
				// 4) 生きていれば移動を反映
				piece->SetBoardPosition(to);
			}
		}
	};

}

// 終了化
void SceneGame::Finalize()
{
	
	if (cameraController != nullptr) {
		delete cameraController;
		cameraController = nullptr;
	}

	if (board != nullptr)
	{
		delete board;
		board = nullptr;
	}

	for (auto p : pieces) delete p;
	pieces.clear();

	network.Finalize();

	delete ai;
	ai = nullptr;

}

// 更新処理
void SceneGame::Update(float elapsedTime)
{
	// ガード処理：ゲーム終了時は処理を停止
	if (isGameOver) {
		// ここに結果表示のロジック（未作成）が入る
		// シーン遷移がないため、結果表示（例：テキスト表示）のみを行う
		if(winnerColor=="white")
			SceneManager::Instance().ChangeScene(new SceneWhiteResult);
		else
			SceneManager::Instance().ChangeScene(new SceneBlackResult);
		return;
	}
	Mouse& mouseCursor = Input::Instance().GetMouse();

	if (mouseCursor.GetButtonDown() & Mouse::BTN_LEFT) {
		POINT cursor = mouseCursor.GetPosition();

		if (board->getCurrentTurn() == "black")
			return;

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

			// 修正ロジック 1: 自己移動の禁止
			if (selectedPos.x == clicked.x && selectedPos.y == clicked.y) {
				// 同じ駒を連打した場合 → 選択解除
				selectedPos = { -1, -1 };
				legalMoves.clear();
				return;
			}

			// 移動先として合法か判定
			bool isLegalMove = false;
			for (auto& move : legalMoves) {
				if (move.x == clicked.x && move.y == clicked.y) {
					isLegalMove = true;
					break; // 合法な移動先を見つけたのでループを抜ける
				}
			}

			
			if (isLegalMove) {
				// slime の位置も更新
				auto piece = FindSlimeAt(selectedPos);

				// 修正: 移動する駒を取得（自傷と死亡判定のため）
				auto movingPiece = board->getPieceAt(selectedPos);
				// 取られる駒があれば、その slime を削除


				// 駒の移動が成功した後、もしカードが選択されていればここで効果を適用
				//if (selectedCardId != -1) {
				//	// 1. カード効果適用
				//	// targetPos は clicked と同じ
				//	bool effectSuccess = ApplyCardEffect(selectedCardId, clicked);

				//	if (effectSuccess) {
				//		// 2. 成功したらクールダウン開始 (破棄)
				//		isCardInUse = true;
				//		cardCooldownTimer = CARD_COOLDOWN_TIME;
				//	}

				//	// 3. 選択解除 (成功/失敗にかかわらず、カードは手札から消えているため)
				//	selectedCardId = -1;

				//	// ... (ターン切り替えなど) ...
				//	return;
				//}
				auto attacker = board->getPieceAt(selectedPos);
				auto defender = board->getPieceAt(clicked); // 取られる駒（nullptrの場合もある）

				// --- 体力ベースの戦闘判定 ---
				if (defender) {

					// 1. 体力を比較
					int attackerHealth = attacker->getHealth();
					int defenderHealth = defender->getHealth();

					// 2. 攻撃側の体力が防御側の体力未満の場合 (負け)
					if (attackerHealth < defenderHealth) {
						// 負け：攻撃側は消滅し、防御側にダメージを与える

						// 描画オブジェクトの削除 (攻撃側)
						RemovePieceAt(selectedPos);
						// 盤面からの駒の削除 (攻撃側)
						board->setPieceAt(selectedPos, nullptr);

						// 防御側に自駒の体力分のダメージを与える
						defender->takeDamage(attackerHealth);

						//  防御側の死亡チェックと削除
						if (defender->getHealth() <= 0) {
							// 防御側も死亡した場合、描画と盤面から削除
							RemovePieceAt(clicked);
							board->setPieceAt(clicked, nullptr);
						}

						// 負けたため、移動処理をスキップし、ターンを終了
						// 負けの場合は移動処理(board->movePiece)は不要

						// 選択を解除し、ターンを切り替える
						selectedPos = { -1, -1 };
						legalMoves.clear();
						board->switchTurn();
						return; // 処理を終了
					}

					// 3. 攻撃側の体力が防御側の体力以上の場合 (勝ち)
					// 勝利：通常通り相手の駒を取得（通常移動のロジックに任せる）
					// 勝利時は、防御側のSlimeを削除する必要があるため、以下の処理を実行
					else {
						RemovePieceAt(clicked); // 取られる駒の Slime を削除

						board->setPieceAt(clicked, nullptr);
					}
				}

				/*auto captured_piece = board->getPieceAt(clicked); // 移動先の駒を取得
				if (captured_piece) {
					// 修正: 取られる駒に対応する Slime オブジェクトをリストから削除する処理を呼び出す
					RemoveSlimeAt(clicked);
				}*/

				// 合法な移動を実行
				board->movePiece(selectedPos, clicked);

				{
					Position currentPos = clicked;
					HealSpot& spot = healSpots[currentPos.y][currentPos.x];
					auto movingPiece = board->getPieceAt(currentPos); // 移動後の駒を取得

					if (movingPiece && spot.isGenerated) {
						std::string pieceColor = movingPiece->getColor();
						bool shouldHeal = false;

						// 1. 回復条件の判定
						if (spot.type == HealType::COMMON) {
							shouldHeal = true;
						}
						else if (spot.type == HealType::BLACK_ONLY && pieceColor == "white") {
							shouldHeal = true;
						}
						else if (spot.type == HealType::WHITE_ONLY && pieceColor == "black") {
							shouldHeal = true;
						}

						if (shouldHeal) {
							int maxHealth = movingPiece->getMaxHealth();
							int currentHealth = movingPiece->getHealth();

							// 回復量の計算 (max(1, maxHealth / 2) の代替)
							int healAmount = maxHealth / 2;
							if (healAmount < 1) {
								healAmount = 1;
							}

							//　setHealth/newHealthの代わりに heal メソッドを使用
							movingPiece->heal(healAmount);

							// 回復マスの消滅（一度使ったら消える場合）
							spot.isGenerated = false;
							spot.type = HealType::NONE;
						}
					}
				}
				
				if (piece) piece->SetBoardPosition(clicked);

				MoveData move{ 1, selectedPos.x, selectedPos.y, clicked.x, clicked.y };
				network.SendMove(move);

				// ----------------------------------------------------
				// 自傷と死亡判定
				// ----------------------------------------------------
				if (movingPiece) {
					// 1. 1ダメージを与える
					movingPiece->takeDamage(1);

					// 2. 体力が0になったら死亡とみなし、盤面から削除
					if (movingPiece->getHealth() <= 0) {
						// 死亡: 盤面（Board）から駒を削除
						board->setPieceAt(clicked, nullptr); // 移動先のマスを空にする

						// 死亡した駒に対応する Slime も描画リストから削除
						RemovePieceAt(clicked);
						board->setPieceAt(clicked, nullptr);
					}
				}

				// 動的な回復マス生成ロジック
				{
					Position movedTo = clicked;
					auto movingPiece = board->getPieceAt(movedTo);

					// 1. 共通マス (y=2 から y=5) への移動チェック
					bool isCommonSpot = (movedTo.y >= 2 && movedTo.y <= 5);

					if (movingPiece && isCommonSpot && !isDynamicHealSpotActive) {

						// 2. 駒の色に基づいてカウンターをインクリメント
						std::string color = movingPiece->getColor();

						if (color == "black") {
							// 移動元の駒の色で判定すべきなので、移動前の駒情報を保持している必要がありますが、
							// 簡略化のため、ここでは移動後の駒の色でカウントします。
							blackMovedToCommonCount++;
						}
						else if (color == "white") {
							whiteMovedToCommonCount++;
						}

						// 3. 生成条件の判定 (黒2回、白2回以上)
						if (blackMovedToCommonCount >= 2 && whiteMovedToCommonCount >= 2) {

							// 4. 生成場所をランダムに決定
							Position targetPos = FindRandomEmptyCommonSpot();

							// 5. 空きマスが見つかり、有効な座標であれば生成
							if (board->isInsideBoard(targetPos)) {

								// HealSpotを生成（共通タイプ）
								GenerateHealSpots();
								isDynamicHealSpotActive = true;

								// カウンターをリセットして、次回の生成条件を待つ
								blackMovedToCommonCount = 0;
								whiteMovedToCommonCount = 0;
							}
						}
					}
				}

				
				// 選択を解除し、ターンを切り替える
				selectedPos = { -1, -1 };
				legalMoves.clear();
				// ゲーム終了判定
					if (!board->isKingPresent("white")) {
						isGameOver = true;
						winnerColor = "black";
					}
					else if (!board->isKingPresent("black")) {
						isGameOver = true;
						winnerColor = "white";
					}

				// ゲームが終了していなければ、ターンを切り替える
				if (!isGameOver) {
					board->switchTurn();
				}
				return;
			}
			else {
				// 不正な場所をクリックした場合

				// 別の自分の駒をクリックしたかチェックし、選択を切り替えるロジックをここに追加できます。
				// 修正ロジック 2: 別の自分の駒をクリックしたかチェック
				auto currentTurn = board->getCurrentTurn();

				if (clickedPiece && clickedPiece->getColor() == currentTurn) {
					// パターンA: 別の自分の駒をクリックした場合 → 選択を切り替える
					selectedPos = clicked;
					legalMoves = clickedPiece->getLegalMoves(*board);
					// 選択を切り替えたので、ここで関数を終了
					return;
				}
				else {
					// パターンB: 何もないマスや敵の駒をクリック → 選択解除
					// シンプルに選択解除のみを行う
					selectedPos = { -1, -1 };
					legalMoves.clear();
				}
			}
			
		}
	}

	if (mouseCursor.GetButtonDown() & Mouse::BTN_LEFT)
	{
		//// --- カードクールダウン処理 ---
		if (isCardInUse) {
			cardCooldownTimer -= elapsedTime;
			if (cardCooldownTimer <= 0.0f) {
				isCardInUse = false;
				cardCooldownTimer = 0.0f;
			}
		}

		// --- 常に手札の0番目（最も新しく引いたカード）を表示対象とする ---
		if (cardManager->getHandSize() > 0) {
			displayHandIndex = 0;
		}
		else {
			displayHandIndex = -1; // 手札が空
		}

		if (mouseCursor.GetButtonDown() & Mouse::BTN_LEFT) {
			POINT cursor = mouseCursor.GetPosition();

			// --- 1. カード使用判定（画面左下のカードをクリック） ---
			// 表示対象のカードがあり、クールダウン中でないか
			if (displayHandIndex != -1 && !isCardInUse) {

				const int CARD_WIDTH = 150;
				const int CARD_HEIGHT = 200;

				if (cursor.x >= CARD_DISPLAY_X && cursor.x < CARD_DISPLAY_X + CARD_WIDTH &&
					cursor.y >= CARD_DISPLAY_Y && cursor.y < CARD_DISPLAY_Y + CARD_HEIGHT)
				{
					// クリックされたら、その手札インデックスを選択状態にする
					selectedHandIndex = displayHandIndex;

					// DebugLog("手札のカードID " + std::to_string(cardManager->getCardInHand(selectedHandIndex).effectId) + " を選択しました。");

					return; // カード操作が完了したので、盤面クリック処理に進まない
				}
			}

			// --- 2. 盤面クリック処理（既存のロジック） ---

			Position clicked = ScreenToBoard(cursor.x, cursor.y);

			if (clicked.isValid()) {

				// ... (駒の移動処理ロジック: isLegalMove を判定) ...

				// --- 3. カード効果の適用（選択状態の場合） ---
				if (selectedHandIndex != -1) {

					// CardManagerにカードの使用と破棄を依頼
					UsedCardInfo usedInfo = cardManager->UseCard(selectedHandIndex);

					// カード使用成功時 (UseCardの内部で hand から削除、isUsedCard = true になっている)
					if (usedInfo.effectId != -1) {

						// 1. カード効果適用 (ターゲットはクリックされたマス)
						bool effectSuccess = ApplyCardEffect(usedInfo.effectId, clicked);

						if (effectSuccess) {
							// 2. 成功したらクールダウン開始
							isCardInUse = true;
							cardCooldownTimer = CARD_COOLDOWN_TIME;
						}
						// ※ effectSuccess が false でも、カードは破棄済み(UseCard内)のため、クールダウンは開始してもよいが、ここでは成功時のみ開始。
					}

					// 選択解除
					selectedHandIndex = -1;
					// displayHandIndex は Update の冒頭で自動で更新される

					return; // カード使用が完了したので、その後の駒選択ロジックへ進まない
				}
			}
		}
	}

	
	MoveData recvMove{};
	if (network.ReceiveMove(recvMove)) {
		Position from{ recvMove.fromX, recvMove.fromY };
		Position to{ recvMove.toX, recvMove.toY };
		board->movePiece(from, to);
		auto slime = FindSlimeAt(from);
		if (slime) slime->SetBoardPosition(to);
	}

	for (auto p : pieces) p->Update(elapsedTime);


	//カメラコントローラー更新処理

	DirectX::XMFLOAT3 Poswhite{ 400.0f,0.0f,300.0f };
	DirectX::XMFLOAT3 Posblack{ 400.0f,0.0f,600.0f };


	DirectX::XMFLOAT3 target;

	//DirectX::XMFLOAT3 target;
	target.x = Poswhite.x;
	target.y = 0.0f;
	target.z = Posblack.z - Poswhite.z;

	target.y += 0.5f;
	cameraController->SetTarget(target);
	cameraController->Update(elapsedTime);

	//ステージ更新処理
	//stage->Update(elapsedTime);
	
	//エフェクトマネージャー更新処理
	EffectManager::Instance().Update(elapsedTime);

	if (!isGameOver) {
		ai->Update(board);
		blackMovedToCommonCount++;
	}

	if (!board->isKingPresent("white")) {
		isGameOver = true;
		winnerColor = "black";
	}
	else if (!board->isKingPresent("black")) {
		isGameOver = true;
		winnerColor = "white";
	}
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
	/*stage->Render(rc, renderer);
	for (auto p : pieces) p->Render(rc, renderer);*/

	

	//カメラパラメータ設定
	Camera& camera = Camera::Instance();
	rc.view = camera.GetView();
	rc.projection = camera.GetProjection();

	// 3Dモデル描画
	{
		for (auto p : pieces) p->Render(rc, renderer);

		// 選択された駒があり、合法手リストが空でなければハイライトを描画
		if (selectedPos.x != -1 && !legalMoves.empty()) {

			for (const auto& move : legalMoves) {

				// 盤面座標 (move.x, move.y) をワールド座標に変換
				DirectX::XMFLOAT4X4 transform;
				DirectX::XMStoreFloat4x4(&transform,
					DirectX::XMMatrixTranslation(
						move.x * 100.0f, // X座標
						2.0f,            // 駒より少し高い位置
						move.y * 100.0f  // Z座標
					)
				);

				// ハイライトモデルを描画
				renderer->Render(rc, transform, highlightModel, ShaderId::Lambert);
			}
		}

		// 回復マスの描画 (仮のロジック)
		for (int y = 0; y < 8; ++y) {
			for (int x = 0; x < 8; ++x) {
				if (healSpots[y][x].isGenerated) {
					// 回復マスのモデルをボード上に描画
					DirectX::XMFLOAT4X4 transform;
					DirectX::XMStoreFloat4x4(&transform,
						DirectX::XMMatrixTranslation(
							x * 100.0f,
							2.0f, // 盤面の高さ
							y * 100.0f
						)
					);
					// マスのタイプに応じて異なるテクスチャや色で描画しても良い
					 modelRenderer->Render(rc, transform, healSpotModel, ShaderId::Lambert);
				}
			}

		}
		//エフェクトマネージャー描画
		EffectManager::Instance().Render(rc.view, rc.projection);
	}

	// 3Dデバッグ描画
	{
		
	}

	// 2Dスプライト描画
	{
		for (auto p : pieces) {
			Position piecePos = p->GetBoardPosition();

			// 1. 選択状態の判定
			bool isSelected = (piecePos.x == selectedPos.x && piecePos.y == selectedPos.y);

			if (isSelected) {
				// 2. 体力情報の取得
				auto chessPiece = board->getPieceAt(piecePos);
				if (!chessPiece) continue;

				int currentHealth = chessPiece->getHealth();
				int maxHealth = chessPiece->getMaxHealth();
				float healthRatio = (float)currentHealth / maxHealth;

				// 3. 3D位置を計算 (駒の頭上に出現させるためのワールド座標)
				DirectX::XMFLOAT3 worldPos = {
					piecePos.x * 100.0f,
					60.0f, // 駒の頭上の高さ (Y軸オフセット)
					piecePos.y * 100.0f
				};

				// 4. ワールド座標をスクリーン座標に変換
				DirectX::XMFLOAT2 screenPos = WorldToScreen(worldPos);

				// 画面外なら描画しない
				if (screenPos.x < 0 || screenPos.y < 0) continue;

				// --- 体力バーの描画パラメータ ---
				const float barWidth = 80.0f;  // スクリーン上のピクセル幅 (最大)
				const float barHeight = 10.0f; // スクリーン上のピクセル高さ

				// 矩形の描画開始座標 (中央揃えのため、中央位置から幅/高さを引く)
				float startX = screenPos.x - barWidth / 2.0f;
				float startY = screenPos.y - barHeight / 2.0f;

				// 5. 体力バーの**背景（枠）**を描画
				// 既に実装した DrawRect を使用
				shapeRenderer->DrawRect(
					rc,
					startX,
					startY,
					barWidth,
					barHeight,
					DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f) // 黒い背景色
				);

				// 6. 現在の体力（塗りつぶし）を描画
				float currentBarWidth = barWidth * healthRatio;

				// 体力に応じた色を設定
				DirectX::XMFLOAT4 barColor;
				if (healthRatio > 0.6f) barColor = DirectX::XMFLOAT4(0.0f, 0.8f, 0.0f, 1.0f);     // 緑
				else if (healthRatio > 0.3f) barColor = DirectX::XMFLOAT4(0.8f, 0.6f, 0.0f, 1.0f);  // オレンジ
				else barColor = DirectX::XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);                          // 赤

				// バーは左から右へ満たされるように描画（startXから開始）
				shapeRenderer->DrawRect(
					rc,
					startX,
					startY,
					currentBarWidth,
					barHeight,
					barColor
				);
			}
		}
	}
}

// GUI描画
void SceneGame::DrawGUI()
{
	//// 1. カードがドローされている場合 (手札)
	//// 手札の0番目があるか、かつクールダウン中でないか
	//if (displayHandIndex != -1 && !isCardInUse) {

	//	const Card& cardToDisplay = cardManager->getCardInHand(displayHandIndex);
	//	std::string cardName = cardToDisplay.name;

	//	// カードの枠/背景を描画
	//	// DrawRect(CARD_DISPLAY_X, CARD_DISPLAY_Y, 150, 200, Color::White);

	//	// カード名を描画
	//	// DrawText(cardName, CARD_DISPLAY_X + 10, CARD_DISPLAY_Y + 10, Color::Black);

	//	// カードの効果説明を描画
	//	// DrawText(cardToDisplay.description, CARD_DISPLAY_X + 10, CARD_DISPLAY_Y + 40, Color::DarkGray);

	//	// 選択中の場合は枠の色を変えるなど
	//	// if (selectedHandIndex == displayHandIndex) {
	//	//     DrawRectBorder(CARD_DISPLAY_X, CARD_DISPLAY_Y, 150, 200, Color::Yellow);
	//	// }
	//}

	//// 2. カード使用中（クールダウン中）の場合 (＝デッキ全体の使用不可状態)
	//if (isCardInUse) {
	//	// ロックされていることを示すUI
	//	float progress = cardCooldownTimer / CARD_COOLDOWN_TIME;

	//	// クールダウンバーの描画 (左下のカード位置にオーバーレイ)
	//	// DrawRect(CARD_DISPLAY_X, CARD_DISPLAY_Y + 220, 150, 20, Color::DarkRed);
	//	// DrawRect(CARD_DISPLAY_X, CARD_DISPLAY_Y + 220, 150 * (1.0f - progress), 20, Color::Red);

	//	// ロックされていることを示すテキスト
	//	// DrawText("カード使用不可 (CD: " + std::to_string((int)ceil(cardCooldownTimer)) + "s)", 
	//	//          CARD_DISPLAY_X, CARD_DISPLAY_Y + 250, Color::Red);
	//}
}

void SceneGame::RemovePieceAt(Position pos)
{
	//// Slimeオブジェクトのリストを走査し、posと一致するものを削除する
	//for (auto it = pieces.begin(); it != pieces.end(); ++it) {
	//	if ((*it)->GetBoardPosition().x == pos.x && (*it)->GetBoardPosition().y == pos.y) {
	//
	//		// 削除前にメモリを解放（ヒープ領域で確保している場合）
	//		delete (*it);
	//
	//		// リストから要素を削除
	//		pieces.erase(it);
	//		return; // 駒は一つしか存在しないので、見つけたら終了
	//	}
	//}

	//board->setPieceAt(pos, nullptr);

	// 描画用リストからも削除
	for (auto it = pieces.begin(); it != pieces.end(); )
	{
		if ((*it)->GetBoardPosition().x == pos.x &&
			(*it)->GetBoardPosition().y == pos.y)
		{
			delete* it; // rawポインタなら delete
			it = pieces.erase(it);
			return; // ★一つ見つけたらループを抜ける (駒は一マスに一つであるため)
		}
		else
		{
			++it;
		}
	}
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

	// 原点補正（盤面の描画開始位置）
	constexpr float boardOriginX = -50.0f; // ← 必要に応じて -50.0f などに調整
	constexpr float boardOriginZ = -50.0f;

	float localX = x - boardOriginX;
	float localZ = z - boardOriginZ;

	// 範囲チェック（盤面サイズ 8x8, 1マス100）
	if (localX < 0 || localX >= 800 || localZ < 0 || localZ >= 800)
		return { -1, -1 };

	// floorで丸め誤差を防止
	int boardX = static_cast<int>(std::floor(localX / 100.0f));
	int boardY = static_cast<int>(std::floor(localZ / 100.0f));

	return { boardX, boardY };
}

//Piece を盤面座標から取得
Piece* SceneGame::FindSlimeAt(Position pos) {
	for (auto p : pieces) {
		if (p->GetBoardPosition().x == pos.x &&
			p->GetBoardPosition().y == pos.y)
			return p;
	}
	return nullptr;
}

DirectX::XMFLOAT2 SceneGame::WorldToScreen(const DirectX::XMFLOAT3& worldPos) const {
	using namespace DirectX;
	Graphics& graphics = Graphics::Instance();
	Camera& camera = Camera::Instance();

	// 1. ワールド座標をビュー・プロジェクション変換
	XMVECTOR worldVec = XMLoadFloat3(&worldPos);
	XMMATRIX view = XMLoadFloat4x4(&camera.GetView());
	XMMATRIX proj = XMLoadFloat4x4(&camera.GetProjection());

	// ワールド座標 → クリップ座標 (NDC)
	XMVECTOR clipVec = XMVector3TransformCoord(worldVec, view * proj);

	// 2. 視錐台の外部チェック (Z < 0 または W < 0 の場合は描画しない)
	float w = XMVectorGetW(clipVec);
	if (w <= 0) return { -9999.0f, -9999.0f }; // 画面外を示す無効な座標

	// 3. クリップ座標 → NDC (正規化デバイス座標)
	float ndcX = XMVectorGetX(clipVec) / w;
	float ndcY = XMVectorGetY(clipVec) / w;

	// 4. NDC → スクリーン座標
	float screenX = (ndcX + 1.0f) * 0.5f * graphics.GetScreenWidth();
	float screenY = (1.0f - ndcY) * 0.5f * graphics.GetScreenHeight(); // Y軸反転

	return { screenX, screenY };
}

void SceneGame::GenerateHealSpots() {

	// 乱数生成器の準備
	std::random_device rd;
	std::mt19937 g(rd());

	// ----------------------------------------------------
	// 1. 各ゾーンのマス座標をリストアップ
	// ----------------------------------------------------

	// <y=0, 1> 黒陣地側 (白駒専用回復マスを生成)
	std::vector<Position> blackTerritorySpots;
	// <y=6, 7> 白陣地側 (黒駒専用回復マスを生成)
	std::vector<Position> whiteTerritorySpots;
	// <y=2, 3, 4, 5> 共通マス (共通回復マスを生成)
	std::vector<Position> commonSpots;

	for (int y = 0; y < 8; ++y) {
		for (int x = 0; x < 8; ++x) {
			Position pos = { x, y };

			if (y <= 1) { // y=0, 1: 黒陣地側
				blackTerritorySpots.push_back(pos);
			}
			else if (y >= 6) { // y=6, 7: 白陣地側
				whiteTerritorySpots.push_back(pos);
			}
			else { // y=2, 3, 4, 5: 共通マス
				commonSpots.push_back(pos);
			}
		}
	}

	// ----------------------------------------------------
	// 2. 各ゾーンで指定された数の回復マスをランダムに生成
	// ----------------------------------------------------

	// A. 黒陣地側 (白専用回復マス: 8個)
	// マスをシャッフル
	std::shuffle(blackTerritorySpots.begin(), blackTerritorySpots.end(), g);
	// 8個選択して設定
	int countA = (blackTerritorySpots.size() < 8) ? (int)blackTerritorySpots.size() : 8;
	for (int i = 0; i < countA; ++i) {
		Position pos = blackTerritorySpots[i];
		healSpots[pos.y][pos.x] = { HealType::WHITE_ONLY, true };
	}

	// B. 白陣地側 (黒専用回復マス: 8個)
	// マスをシャッフル
	std::shuffle(whiteTerritorySpots.begin(), whiteTerritorySpots.end(), g);
	// 8個選択して設定
	int countB = (whiteTerritorySpots.size() < 8) ? (int)whiteTerritorySpots.size() : 8;
	for (int i = 0; i < countB; ++i) {
		Position pos = whiteTerritorySpots[i];
		healSpots[pos.y][pos.x] = { HealType::BLACK_ONLY, true };
	}

	// C. 共通マス (共通回復マス: 16個)
	// マスをシャッフル
	std::shuffle(commonSpots.begin(), commonSpots.end(), g);
	// 16個選択して設定
	int countC = (commonSpots.size() < 16) ? (int)commonSpots.size() : 16;
	for (int i = 0; i < countC; ++i) {
		Position pos = commonSpots[i];
		healSpots[pos.y][pos.x] = { HealType::COMMON, true };
	}

}

//void SceneGame::DrawNewCard(CardManager* cardManager) {
//	// CardManager (または Card.h に定義された全カードリスト) からランダムにカードIDを選択
//	// ここでは、効果ID 1から9までをランダムに選ぶと仮定します。
//	// ID 0 (攻撃) は通常移動に相当するため除外。
//
//	if (cardManager->isHandFull()) {
//		return; // 手札が既に満杯なら何もしない
//	}
//
//	// 1. CardManagerに実際のドロー処理を依頼
//	cardManager->DrawCard();
//
//	// 2. SceneGameの描画用変数に、ドローされたカードの情報を反映
//	// (手札の最後のカードのIDを取得すると仮定)
//	if (!cardManager->getHand().empty()) {
//		const Card& drawnCard = cardManager->getHand().back();
//		this->drawnCardId = drawnCard.id; // カード構造体から ID を取得
//	}
//	else {
//		this->drawnCardId = -1; // ドローできなかった
//	}
//}

void SceneGame::ApplyPersistentEffect(const ActiveEffect& effect)
{
	// 発動した効果に応じてゲーム状態を変化させる

	auto targetPiece = board->getPieceAt(effect.targetPos);
	 Position target = effect.targetPos;

	switch (effect.sourceEffectId) {
	case 3: // 悠久の盟約: 自身の駒全体の体力を2回復
	{
		 //effect.ownerColor のプレイヤーの全ての駒を探し、2回復させる
		 for (auto piece : pieces) {
		     /*if (piece->getColor() == effect.ownerColor) {
		         piece->heal(2);
		     }*/
		 }
		//std::cout << "悠久の盟約 (ID 3) が発動: " << effect.ownerColor << "側の駒全体を2回復" << std::endl;
	}
	break;

	case 6: // 破滅の刻印: 自身を中心とする周囲8マスに2ダメージ
	{
		// targetPiece (刻印を付与された駒) を中心に周囲8マス（3x3の範囲）内の全駒にダメージを与える
		// for (int dy = -1; dy <= 1; ++dy) {
		//     for (int dx = -1; dx <= 1; ++dx) {
		//         Position damagePos = {effect.targetPos.x + dx, effect.targetPos.y + dy};
		//         // if (board->isInsideBoard(damagePos) && (dx != 0 || dy != 0)) {
		//         //     auto victim = board->getPieceAt(damagePos);
		//         //     if (victim) victim->takeDamage(2);
		//         // }
		//     }
		// }
		//std::cout << "破滅の刻印 (ID 6) が発動: ターゲット周囲に2ダメージ" << std::endl;
	}
	break;

	default:
		// 未知の持続効果
		break;
	}
}

bool SceneGame::IsTargetPiece(Position pos, const std::string& requiredColor) const
{
	auto piece = board->getPieceAt(pos);
	if (!piece) return false;
	return piece->getColor() == requiredColor;
}

bool SceneGame::ApplyCardEffect(int effectId, Position targetPos)
{
	// 現在のターンの色
	std::string currentTurn = board->getCurrentTurn();
	std::string enemyColor = (currentTurn == "white") ? "black" : "white";

	// ターゲット駒 (このカードは空きマスにも打てる可能性があるため、ここではターゲット駒の存在は必須ではない)
	 auto targetPiece = board->getPieceAt(targetPos);

	// 乱数生成器 (他の効果用)
	// ...

	switch (effectId) {
		// ... (既存の case 0, 2, 3, 4, 5, 6, 7, 8, 9) ...

	case 1: // 焦土の罠 (Trap): 指定した場所の前方 2x3 マスの駒に 1 ダメージ
	{
		// ターゲット位置がボード内か確認
		if (!board->isInsideBoard(targetPos)) {
			return false; // 無効な位置
		}

		// 1. ダメージエリアの定義
		int damage = 1;

		// 進行方向の定義 (白: y軸減少方向へ前方, 黒: y軸増加方向へ前方)
		// 白 (y=7, 6 から y=0, 1 へ移動) の場合、前方マスは y - 1, y - 2
		// 黒 (y=0, 1 から y=7, 6 へ移動) の場合、前方マスは y + 1, y + 2
		const int direction = (currentTurn == "white") ? -1 : 1;

		// 展開エリア: 前方2マス (y+direction*1, y+direction*2)、横幅3マス (x-1, x, x+1)
		// 前方 2 マス (dy = 1, 2)
		for (int dy = 1; dy <= 2; ++dy) {
			// 横幅 3 マス (dx = -1, 0, 1)
			for (int dx = -1; dx <= 1; ++dx) {

				// ダメージを与えるマスの座標を計算
				Position damagePos = {
					targetPos.x + dx,
					targetPos.y + (direction * dy) // 進行方向へ展開
				};

				// 2. ボード内チェックとダメージ適用
				if (board->isInsideBoard(damagePos)) {
					auto victim = board->getPieceAt(damagePos);

					if (victim) {
						// 駒が存在する場合、ダメージを与える
						victim->takeDamage(damage);

						// 死亡チェック (takeDamage後に health <= 0 になったら、リストから削除する処理が必要)
						 /*if (victim->getHealth() <= 0) { 
						     board->RemovePieceAt(victim);
						 }*/
					}
				}
			}
		}

		// トラップ設置は成功したとみなす
		return true;
	}

	case 2: // 生命の祝福 (Buff): 自身の駒単体の体力を3回復
	{
		const int healAmount = 3;

		// 1. ターゲット駒が存在し、かつそれが自駒であるかを確認
		if (targetPiece && IsTargetPiece(targetPos, currentTurn)) {

			// 2. 体力を回復
			targetPiece->heal(healAmount);

			
			return true; // 効果適用成功
		}

		// ターゲットが無効（駒がいない、または敵駒）
		return false;
	}

	case 4: // 石化の鎖 (Debuff): 相手の駒単体の移動を制限 (キング使用不可)
	{
		// 1. ターゲット駒が存在し、かつそれが敵駒であるかを確認
		if (targetPiece && IsTargetPiece(targetPos, enemyColor)) {

			// 2. キングではないかを確認
			if (targetPiece->getType() == "King") {
				// DebugLog("石化の鎖はキングに使用できません。");
				return false; // キングには使用不可
			}

			// 3. 駒の移動を制限
			targetPiece->setImmobilized(true);

			// DebugLog(targetPiece->getColor() + " の " + targetPiece->getType() + " に移動制限を適用しました。");
			return true; // 効果適用成功
		}

		// ターゲットが無効（駒がいない、自駒、または無効なカード使用）
		return false;
	}

	default:
		return false;
	}
}

//　駒が置かれておらず、かつ回復マスが生成されていない共通マスをランダムに選ぶ
Position SceneGame::FindRandomEmptyCommonSpot() const
{
	// 1. 候補となる空いている共通マスの座標を収集
	std::vector<Position> candidates;

	for (int y = 2; y <= 5; ++y) {
		for (int x = 0; x < 8; ++x) {
			Position pos = { x, y };

			// 共通マスに駒が置かれていないか？ (nullptr)
			bool isSpotEmpty = (board->getPieceAt(pos) == nullptr);

			// 共通マスにまだ回復マスが生成されていないか？ (!isGenerated)
			bool isHealSpotEmpty = !healSpots[y][x].isGenerated;

			if (isSpotEmpty && isHealSpotEmpty) {
				candidates.push_back(pos);
			}
		}
	}

	// 2. 候補マスが一つもない場合の処理
	if (candidates.empty()) {
		return { -1, -1 };
	}

	// 3. 候補の中から一つをランダムに選択
	std::random_device rd;
	std::mt19937 g(rd());

	std::uniform_int_distribution<> distrib(0, candidates.size() - 1);
	int randomIndex = distrib(g);

	return candidates[randomIndex];
}