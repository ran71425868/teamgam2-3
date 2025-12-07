#include <random>
#include <algorithm> // std::shuffle用
#include "System/Graphics.h"
#include"System/Audio.h"
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

	BGM = Audio::Instance().LoadAudioSource("Data/Sound/Game.wav");

	// 視覚オブジェクトを生成し、対応する論理オブジェクトとリンクさせるヘルパー関数
	auto createAndLinkPiece = [&](const std::string& color, Position pos, const std::string& type) {
		Piece* p = new Piece(color, pos, type);

		// 盤面上のChessPieceを取得し、Pieceに設定
		// Board::getPieceAt が ChessPiece* を返すことを想定
		ChessPiece* logicPiece = board->getPieceAt(pos).get();
		if (logicPiece) {
			p->SetLogicPiece(logicPiece);
		}

		pieces.push_back(p);
		};

	// 駒を初期配置（例：白と黒のスライム）
	// ポーン
	for (int i = 0; i <= 7; i++)
	{
		createAndLinkPiece("white", { i, 1 }, "pawn"); // 白　手前
		createAndLinkPiece("black", { i, 6 }, "pawn"); // 黒　奥
	}
	
	// king
	createAndLinkPiece("white", { 4, 0 }, "king");
	// rook
	createAndLinkPiece("white", { 0, 0 }, "rook");
	createAndLinkPiece("white", { 7, 0 }, "rook");
	// knight
	createAndLinkPiece("white", { 1, 0 }, "knight");
	createAndLinkPiece("white", { 6, 0 }, "knight");
	// bishop
	createAndLinkPiece("white", { 2, 0 }, "bishop");
	createAndLinkPiece("white", { 5, 0 }, "bishop");
	// queen
	createAndLinkPiece("white", { 3, 0 }, "queen");

	// king
	createAndLinkPiece("black", { 4, 7 }, "king");
	// rook
	createAndLinkPiece("black", { 0, 7 }, "rook");
	createAndLinkPiece("black", { 7, 7 }, "rook");
	// knight
	createAndLinkPiece("black", { 1, 7 }, "knight");
	createAndLinkPiece("black", { 6, 7 }, "knight");
	// bishop
	createAndLinkPiece("black", { 2, 7 }, "bishop");
	createAndLinkPiece("black", { 5, 7 }, "bishop");
	// queen
	createAndLinkPiece("black", { 3, 7 }, "queen");

	// CardEffectProcessor の初期化
	// ApplyDamageToAllEnemyPieces をラムダ式でキャプチャし、コールバックとして渡す
	auto damageCallback = [this](const std::string& enemyColor, int damage) {
		this->ApplyDamageToAllEnemyPieces(enemyColor, damage);
	};

	cardProcessor = std::make_unique<CardEffectProcessor>(
		board,
		&pieces,
		&whiteCardManager,
		&blackCardManager,
		damageCallback);

	// CardManagerの初期化 (SceneGame::cardManagerがポインタと仮定)
	cardManager = new CardManager();

	// 効果IDごとのカード画像をロード
	for (int i = 0; i < 10; ++i) {
		std::string filename = "Data/Sprite/Card_" + std::to_string(i) + ".png";
		cardSprites[i] = new Sprite(filename.c_str());
	}

	// ここから追加: 効果IDごとのカード説明画像をロード
	for (int i = 0; i < 10; ++i) {
		std::string filename = "Data/Sprite/Card_Desc_" + std::to_string(i) + ".png";
		cardDescriptionSprites[i] = new Sprite(filename.c_str());
	}

	//カメラ初期設定
	Graphics& graphics = Graphics::Instance();
	Camera& camera = Camera::Instance();
	camera.SetLookAt(
		DirectX::XMFLOAT3(0,10,-10),//視点
		DirectX::XMFLOAT3(0,0,0),//注視点
		DirectX::XMFLOAT3(0,1,0));//上方向
	
	camera.SetPerspectiveFov(
		DirectX::XMConvertToRadians(45),//視野角
		graphics.GetScreenWidth() / graphics.GetScreenHeight(),//画面アスペクト比
		0.1f,//クリップ距離(近)
		10000.0f);//クリップ距離(遠)

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

	check= new Sprite("Data/Sprite/check1.png");
	BGM->Play(false);
	BGM->SetVolume(0.4f);

	deleteEffect = new Effect("Data/Effect/Blow11.efk");
	flame= new Effect("Data/Effect/flame.efk");
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

	if (cardManager != nullptr) {
		delete cardManager;
		cardManager = nullptr;
	}

	for (int i = 0; i < 10; ++i) {
		if (cardSprites[i] != nullptr) {
			delete cardSprites[i];
			cardSprites[i] = nullptr;
		}
	}

	for (int i = 0; i < 10; ++i) {
		if (cardDescriptionSprites[i] != nullptr) {
			delete cardDescriptionSprites[i];
			cardDescriptionSprites[i] = nullptr;
		}
	}

	for (auto p : pieces) delete p;
	pieces.clear();

	delete BGM;
	delete ai;
	delete deleteEffect;

	ai = nullptr;
}

// 更新処理
void SceneGame::Update(float elapsedTime)
{
	// ガード処理：ゲーム終了時は処理を停止
	if (isGameOver && endtimer >= 200.0f) {
		// ここに結果表示のロジック（未作成）が入る
		// シーン遷移がないため、結果表示（例：テキスト表示）のみを行う
		BGM->Stop();
		if (winnerColor == "white")
			SceneManager::Instance().ChangeScene(new SceneWhiteResult);
		else
			SceneManager::Instance().ChangeScene(new SceneBlackResult);
		return;
	}

if(!isGameOver){
	// --- カードクールダウン処理 ---
	if (isCardInUse) {
		cardCooldownTimer -= elapsedTime;
		if (cardCooldownTimer <= 0.0f) {
			isCardInUse = false;
			cardCooldownTimer = 0.0f;
		}
	}

	// =======================================================
	// ★ [最優先] ターン切り替わり時の持続効果処理の実行
	// =======================================================
	std::string currentTurn = board->getCurrentTurn();

	if (currentTurn != lastTurnColor) {

		// ActiveEffectManager にターン経過を通知し、効果を処理させる
		ActiveEffectManager::GetInstance().ProcessTurnEffects(
			currentTurn,
			[this](const ActiveEffect& effect) {
				ApplyPersistentEffect(effect); // ID 10 がここで実行され、移動不可が解除される
			});

		lastTurnColor = currentTurn; // 前ターンを更新
	}

	Mouse& mouseCursor = Input::Instance().GetMouse();
	GamePad& gamePad = Input::Instance().GetGamePad();

	// --- キーボード入力によるカード選択/決定 ---
	if (!isCardInUse && cardManager->getHandSize() > 0) {
		size_t handSize = cardManager->getHandSize();

		// Aキー (左へ選択移動)
		if (gamePad.GetButtonDown() & GamePad::BTN_LEFT) {
			selectedHandIndex = (selectedHandIndex > 0) ? selectedHandIndex - 1 : 0;
		}

		// Dキー (右へ選択移動)
		if (gamePad.GetButtonDown() & GamePad::BTN_RIGHT) {
			selectedHandIndex = (selectedHandIndex < handSize - 1) ? selectedHandIndex + 1 : (int)handSize - 1;
		}

		// マウス右クリック (カード決定・使用開始)
		if (mouseCursor.GetButtonDown() & Mouse::BTN_RIGHT)
		{
			POINT cardcursor = mouseCursor.GetPosition();
			
			// --- 1. 盤面クリック座標の取得 ---
			Position cardclicked = ScreenToBoard(cardcursor.x, cardcursor.y);

			// --- 2. カード効果の適用（右クリックで決定済みの場合） ---
			// selectedHandIndex が 0以上の値で固定されている = ターゲット選択フェーズ
			if (selectedHandIndex != -1) {

				
				// 1. カードが使用済みでないか、インデックスが有効かチェック
				if (!cardManager->isCardUsable(selectedHandIndex)) {
					// カードが使用済み（isUsedCard = true）の場合、またはインデックスが無効な場合は、
					// カード使用処理全体をスキップし、リターンする
					return;
				}
				// 決定したカードの効果IDを取得し、状態をリセットする前に保存
				int effectId = cardManager->getCardInHand(selectedHandIndex).effectId;
				deleteEffect->Play({ -240.0f,0.0f,0.0f }, 30.0f);
				//  即時発動カード (ID 0, 5, 8, 9) の処理を CardEffectProcessor に委譲 (★ID: 5 を追加)
				if (effectId == 0 || effectId == 5 || effectId == 8 || effectId == 9)
				{

					cardProcessor->ProcessInstantCard(
						effectId,
						board->getCurrentTurn(),
						cardManager,
						selectedHandIndex,
						isCardInUse,
						cardCooldownTimer,
						CARD_COOLDOWN_TIME);

					// 絶対の雷 (ID 8) はカード処理後にターン終了を伴う
					if (effectId == 8) {
						board->switchTurn();
					}

					selectedHandIndex = -1; // ハイライト解除
					return;
				}

				// 焦土の罠(ID: 1) の処理を ID 7 の前に移動・追加
				if (effectId == 1) {
					// ターゲット選択フェーズへ移行 (マス選択待ち)
					selectedCardEffectId = effectId;
					cardManager->UseCard(selectedHandIndex); // 破棄

					// 状態を「マス選択待ち」に設定
					currentCardEffectState = CardEffectState::AWAITING_SQUARE_TARGET;

					selectedHandIndex = -1;

					// 合法なターゲットマスを計算し、リストに格納
					CalculateCardTargetCandidates(effectId);

					return;
				}

				// 次元の扉 (ID: 7) の処理
				if (effectId == 7) {
					// ターゲット選択フェーズへ移行 (ワープ元駒の選択待ち)
					// 1. 選択中のカードの効果IDを保存
					selectedCardEffectId = effectId;

					// 2. カードを手札から削除
					cardManager->UseCard(selectedHandIndex); // 破棄

					// 3. 状態を「ワープ元駒選択待ち」に設定
					currentCardEffectState = CardEffectState::DIMENSIONAL_GATE_SELECT_PIECE;

					// 4. GUIハイライト用のインデックスはリセット
					selectedHandIndex = -1;

					return;
				}

				// ターゲット駒の選択が必要な効果かチェック (ID 2, 3, 4, 6)
				if (effectId == 2 || effectId == 3 || effectId == 4 || effectId == 6)
				{
					// ターゲット選択フェーズへ移行
					// 1. 選択中のカードの効果IDを保存
					selectedCardEffectId = effectId;

					// 2. カードを手札から削除（先に削除しておき、発動失敗時は何もしない）
					cardManager->UseCard(selectedHandIndex); // 破棄

					// 3. 状態を「駒選択待ち」に設定
					currentCardEffectState = CardEffectState::AWAITING_PIECE_SELECTION;

					// 4. GUIハイライト用のインデックスはリセット
					selectedHandIndex = -1;

					// 5. ★追加: 合法なターゲット駒を計算し、リストに格納
					CalculateCardTargetCandidates(effectId); // ← 新しいヘルパー関数を呼び出す

					// 右クリックで決定後のクリックはカード使用に専念させるため、ここでリターン
					return;
				}
			}
		}
	}

	// --- カード使用後、クールダウンが終了した場合 ---
	if (!isCardInUse && cardCooldownTimer > 0.0f) {
		// カード使用後のクールダウンが終了したが、ターンはまだ続行
		// ここでは特に何もしない
	}

	// --- プレイヤーの駒移動が完了し、ターンを終了する判定箇所 ---
	if (board->getCurrentTurn() == "black") {

		// 1. CardManagerの状態をリセットし、1枚ドロー
		cardManager->StartTurn();

		// 2. ターン切り替え

		// 3. プレイヤー側の場合、選択中のカードをリセット
		selectedHandIndex = 0;
	}

	if (mouseCursor.GetButtonDown() & Mouse::BTN_LEFT) {
		POINT cursor = mouseCursor.GetPosition();

		if (board->getCurrentTurn() == "black")
			return;

		timer = 0;
		// 1. ボード座標への変換とボード外チェック
		Position clicked = ScreenToBoard(cursor.x, cursor.y);
		if (!board->isInsideBoard(clicked)) return; // ボード外なら何もしない

		// =======================================================
		// ★ [最優先] カード効果のターゲット選択ロジック
		// =======================================================

		// 状態が NONE でない場合、カード効果のターゲット選択処理に専念する
		if (currentCardEffectState != CardEffectState::NONE)
		{
			// ID 1 のターゲット選択の場合 (AWAITING_SQUARE_TARGET)
			if (currentCardEffectState == CardEffectState::AWAITING_SQUARE_TARGET) {
				// ApplyTargetedEffect に処理を委譲（ID 1 はターゲットマスのみを使用）
				if (cardProcessor->ApplyTargetedEffect(selectedCardEffectId, clicked, board->getCurrentTurn())) {
					// カード使用成功時の共通処理
					isCardInUse = true;
					cardCooldownTimer = CARD_COOLDOWN_TIME;

					// 成功時: 状態をリセット
					currentCardEffectState = CardEffectState::NONE;
					selectedCardEffectId = -1;

					// ターゲット候補をクリア
					cardTargetCandidates.clear();
				}
				// 失敗時は状態維持 (再選択待ち)
				return; // カードターゲット選択に専念するため、駒の移動処理には進まない
			}

			// ID 2, 3, 4, 6 のターゲット選択の場合 (AWAITING_PIECE_SELECTION)
			if (currentCardEffectState == CardEffectState::AWAITING_PIECE_SELECTION)
			{
				// ApplyTargetedEffect は、成功した場合に true を返し、カード使用後のクールダウン処理などを CardEffectProcessor 側で実施
				if (cardProcessor->ApplyTargetedEffect(selectedCardEffectId, clicked, board->getCurrentTurn())) {
					// カード使用成功時の共通処理
					isCardInUse = true;
					cardCooldownTimer = CARD_COOLDOWN_TIME;
					
					// 成功時: 状態をリセット
					currentCardEffectState = CardEffectState::NONE;
					selectedCardEffectId = -1;

					// ★追加: ターゲット候補をクリア
					cardTargetCandidates.clear(); // ← クリア
				}
				// 失敗時は状態維持 (再選択待ち)
				return; // カードターゲット選択に専念するため、駒の移動処理には進まない
			}

			// DIMENSIONAL_GATE_SELECT_PIECE (ID 7) の処理 (ワープ元駒の選択待ち)
			if (currentCardEffectState == CardEffectState::DIMENSIONAL_GATE_SELECT_PIECE) {
				// CardEffectProcessor に駒の選択ロジックを委譲
				// selectedPieceForEffect に選択された Piece* を一時保存
				Piece* pieceToMove = FindSlimeAt(clicked); // 描画オブジェクトを取得

				if (pieceToMove && pieceToMove->getColor() == board->getCurrentTurn()) {
					// 1. 選択した駒 (Piece*) を一時保存
					selectedPieceForEffect = pieceToMove;
					// 2. 論理駒 (ChessPiece*) が存在するかチェック
					ChessPiece* logicPiece = board->getPieceAt(clicked).get();
					if (logicPiece) {
						// 3. 成功時: 状態を「ワープ先マス選択待ち」に移行
						currentCardEffectState = CardEffectState::DIMENSIONAL_GATE_SELECT_TARGET;
						// ワープ先候補の計算と格納 (全ての空きマス)
						dimensionalGateTargets.clear();
						for (int y = 2; y < 6; ++y) {
							for (int x = 0; x < 8; ++x) {
								Position p = { x, y };
								// 駒が置かれていないマスを候補とする (前回の修正で空きマスのみワープ可となったため)
								if (!board->getPieceAt(p)) {
									dimensionalGateTargets.push_back(p);
								}
							}
						}
					}
				}
				// 失敗時は状態維持 (再選択待ち)
				return; // カードターゲット選択に専念するため、移動処理には進まない
			}

			// DIMENSIONAL_GATE_SELECT_TARGET (ID 7) の処理 (ワープ先マスの選択待ち)
			if (currentCardEffectState == CardEffectState::DIMENSIONAL_GATE_SELECT_TARGET) {
				// CardEffectProcessor にターゲットマスの選択ロジックを委譲

				// ApplyDimensionalGate は、成功した場合に true を返す（駒の移動と状態リセットを含む）
				// この関数は Board::movePiece を内部で呼び出す必要があります。
				if (cardProcessor->ApplyDimensionalGate(selectedCardEffectId, clicked, board->getCurrentTurn(), selectedPieceForEffect)) {

					// 描画オブジェクトの位置を更新
					if (selectedPieceForEffect) {
						selectedPieceForEffect->SetBoardPosition(clicked);
					}

					// 成功時: 状態をリセット
					currentCardEffectState = CardEffectState::NONE;
					selectedCardEffectId = -1;
					selectedPieceForEffect = nullptr; // 一時保存駒もリセット

					// ワープ先ハイライトをクリア
					dimensionalGateTargets.clear();

					// カード使用によるターン終了（次元の扉の効果として）
					//board->switchTurn();
				}
				// 失敗時は状態維持 (再選択待ち)
				return; // カードターゲット選択に専念するため、移動処理には進まない
			}
		}

		auto clickedPiece = board->getPieceAt(clicked);
		// --- 2. 駒が選択されていない場合 (selectedPos.x == -1) ---
		if (selectedPos.x == -1) {
			// 自分の駒をクリックした場合、それを選択する
			if (clickedPiece) {
				auto color = clickedPiece->getColor();
				auto currentTurn = board->getCurrentTurn();

				// 駒が移動不可状態ではないかチェック
				if (clickedPiece->isImmobilized()) {
					return; // 選択をキャンセル
				}

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
						deleteEffect->Play({ selectedPos.x * 100.0f,100.0f,selectedPos.y * 100.0f }, 30.0f);
						// 描画オブジェクトの削除 (攻撃側)
						RemovePieceAt(selectedPos);
						// 盤面からの駒の削除 (攻撃側)
						board->setPieceAt(selectedPos, nullptr);

						// 防御側に自駒の体力分のダメージを与える
						defender->takeDamage(attackerHealth);

						//  防御側の死亡チェックと削除
						if (defender->getHealth() <= 0) {
							// 防御側も死亡した場合、描画と盤面から削除
							deleteEffect->Play({ clicked.x * 100.0f,100.0f,clicked.y * 100.0f }, 30.0f);
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
						deleteEffect->Play({ clicked.x * 100.0f,100.0f,clicked.y * 100.0f }, 30.0f);
						RemovePieceAt(clicked); // 取られる駒の Slime を削

						board->setPieceAt(clicked, nullptr);
					}
				}

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
						deleteEffect->Play({ selectedPos.x * 100.0f,100.0f,selectedPos.y * 100.0f }, 100.0f);
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

	target.x = Poswhite.x;
	target.y = 0.0f;
	target.z = Posblack.z - Poswhite.z;

	target.y += 0.5f;
	cameraController->SetTarget(target);
	cameraController->Update(elapsedTime);

	//エフェクトマネージャー更新処理
	EffectManager::Instance().Update(elapsedTime);

	if (board->getCurrentTurn() == "black")
	{
		timer++;
	}

	if (isGameOver)
		endtimer++;

	if (!isGameOver&&timer>=120) {
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

	//カメラパラメータ設定
	Camera& camera = Camera::Instance();
	rc.view = camera.GetView();
	rc.projection = camera.GetProjection();

	// 3Dモデル描画
	{
		for (auto p : pieces) p->Render(rc, renderer);

		board->Render(rc, renderer);

		// 選択された駒があり、合法手リストが空でなければハイライトを描画
		if (selectedPos.x != -1 && !legalMoves.empty()) {

			for (const auto& move : legalMoves) {

				// 盤面座標 (move.x, move.y) をワールド座標に変換
				DirectX::XMFLOAT4X4 transform;
				DirectX::XMStoreFloat4x4(&transform,
					DirectX::XMMatrixTranslation(
						move.x * 100.0f, // X座標
						2.0f,            // 駒より少し高い位置
						move.y * 100.0f));// Z座標

				// ハイライトモデルを描画
				renderer->Render(rc, transform, highlightModel, ShaderId::Lambert);
			}
		}

		// --- 2.カードターゲット候補のハイライト ---
		if ((currentCardEffectState == CardEffectState::AWAITING_PIECE_SELECTION || currentCardEffectState == CardEffectState::AWAITING_SQUARE_TARGET) && !cardTargetCandidates.empty()) {

			for (const auto& pos : cardTargetCandidates) {
				// 盤面座標 (move.x, move.y) をワールド座標に変換
				DirectX::XMFLOAT4X4 transform;
				DirectX::XMStoreFloat4x4(&transform,
					DirectX::XMMatrixTranslation(
						pos.x * 100.0f, // X座標
						2.0f,            // 駒より少し高い位置
						pos.y * 100.0f ));// Z座標

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
							y * 100.0f));
					// マスのタイプに応じて異なるテクスチャや色で描画しても良い
					 modelRenderer->Render(rc, transform, healSpotModel, ShaderId::Lambert);
				}
			}
		}

		if (currentCardEffectState == CardEffectState::DIMENSIONAL_GATE_SELECT_TARGET)
		{
			// dimensionalGateTargets に格納された全ての空きマスを描画
			for (const auto& pos : dimensionalGateTargets)
			{
				float x = pos.x;
				float z = pos.y;
				// 盤面座標 (move.x, move.y) をワールド座標に変換
				DirectX::XMFLOAT4X4 transform;
				DirectX::XMStoreFloat4x4(&transform,
					DirectX::XMMatrixTranslation(
						x * 100.0f, // X座標
						2.0f,            // 駒より少し高い位置
						z * 100.0f));// Z座標

				// ハイライトモデルを描画
				renderer->Render(rc, transform, highlightModel, ShaderId::Lambert);
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

			if (1) {
				// 2. 体力情報の取得
				int currentHealth = p->getHealth(); // Pieceオブジェクトから直接取得
				int maxHealth = p->getMaxHealth();   // Pieceオブジェクトから直接取得

				if (currentHealth == 0) continue; // 論理駒がリンクされていないか、HPが0なら描画しない

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
					DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f)); // 黒い背景色

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
					barColor);
			}
		}
	}
	if (board->isKingInCheck("black") || board->isKingInCheck("white"))
	{
		check->Render(rc,
			500, -200, 0, 800, 500,
			0,
			1, 1, 1, 1);
	}
}

// GUI描画
void SceneGame::DrawGUI()
{
	// Graphicsインスタンスを取得
	Graphics& graphics = Graphics::Instance();

	// 描画に必要なレンダラーとコンテキスト情報を取得/作成
	ShapeRenderer* shapeRenderer = graphics.GetShapeRenderer();

	// RenderContextを作成
	RenderContext rc = {
		graphics.GetDeviceContext(), // ID3D11DeviceContext* を取得
		graphics.GetRenderState()    // RenderState* を取得
	};

	// --- 0. 選択中のカードの効果説明文の描画 ---
	size_t handSize = cardManager->getHandSize();
	// カードが選択されており、かつカードが使用中でない場合
	if (!isCardInUse && handSize > 0 && selectedHandIndex != -1) {

		// 選択中のカード情報を取得
		const Card& selectedCard = cardManager->getCardInHand(selectedHandIndex);

		// 描画位置とサイズの定義 (画面左上隅を起点とする)
		const float CARD_DESC_X = 50.0f; // 描画開始 X 座標
		const float CARD_DESC_Y = 50.0f; // 描画開始 Y 座標
		const float DESC_WIDTH = 300.0f; // 説明画像の幅
		const float DESC_HEIGHT = 400.0f; // 説明画像の高さ

		// 説明文の背景として黒い矩形を描画 (読みやすさのため)
		shapeRenderer->DrawRect(
			rc,
			CARD_DESC_X - 10,  // X座標を少しずらしてパディング
			CARD_DESC_Y - 10,  // Y座標を少しずらしてパディング
			DESC_WIDTH + 20.0f, // 画像サイズに合わせて幅を設定
			DESC_HEIGHT + 20.0f, // 画像サイズに合わせて高さを設定
			DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.7f));// 黒、半透明

		// cardDescriptionSprites を使用
		int effectId = selectedCard.effectId;
		Sprite* descriptionSprite = nullptr;

		// effectId を使って対応する説明スプライトを取得
		if (effectId >= 0 && effectId < 10) {
			descriptionSprite = cardDescriptionSprites[effectId];
		}

		if (descriptionSprite != nullptr) {
			descriptionSprite->Render(
				rc,
				CARD_DESC_X, CARD_DESC_Y, // dx, dy (描画開始位置)
				0.0f, // dz
				DESC_WIDTH, DESC_HEIGHT, // 説明画像用のサイズ
				0.0f, // angle
				1.0f, 1.0f, 1.0f, 1.0f);// r, g, b, a
		}
	}

	// --- 1. 手札の描画 ---
	for (int i = 0; i < handSize; ++i) {

		// カードの描画位置を計算 (横並び)
		int cardX = CARD_START_X + (CARD_WIDTH + CARD_SPACING) * i;
		int cardY = CARD_START_Y;

		// 現在の手札インデックスのカード情報を取得
		const Card& card = cardManager->getCardInHand(i);
		int effectId = card.effectId;
		Sprite* spriteToRender = nullptr;

		// 効果IDが有効範囲内かチェックし、対応するスプライトを選択
		if (effectId >= 0 && effectId < 10) {
			spriteToRender = cardSprites[effectId];
		}

		// --- A. カードの描画 (Sprite::Renderを使用) ---
		if (spriteToRender != nullptr) {
			spriteToRender->Render(
				rc,
				(float)cardX, (float)cardY, // dx, dy (描画開始位置)
				0.0f,// dz
				(float)CARD_WIDTH, (float)CARD_HEIGHT, // ★修正済みの定数を使用
				0.0f,// angle
				1.0f, 1.0f, 1.0f, 1.0f);// r, g, b, a
		}
	
		// --- B. 選択中のハイライト描画 ---
		if (!isCardInUse && i == selectedHandIndex) {
			// 選択中のカードの周囲に枠を描画
			shapeRenderer->DrawRectBorder(rc,cardX, cardY, CARD_WIDTH, CARD_HEIGHT,
				DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), 5.0f);
		}

		// --- C. カード名とIDのテキスト描画 --- 
	}
}

void SceneGame::RemovePieceAt(Position pos)
{
	//board->setPieceAt(pos, nullptr);

	// 描画用リストからも削除
	for (auto it = pieces.begin(); it != pieces.end(); )
	{
		if ((*it)->GetBoardPosition().x == pos.x &&
			(*it)->GetBoardPosition().y == pos.y)
		{
			delete* it; // rawポインタなら delete
			it = pieces.erase(it);
			return; // 一つ見つけたらループを抜ける (駒は一マスに一つであるため)
		}
		else
		{
			++it;
		}
	}
}

void SceneGame::ApplyDamageToAllEnemyPieces(const std::string& enemyColor, int damage)
{
	// ダメージ適用後に削除される駒のリスト
	std::vector<Piece*> piecesToRemove;

	// 1. 全ての駒をチェック
	for (Piece* piece : pieces)
	{
		if (piece == nullptr) continue;

		// 2. 敵駒であるかチェック
		if(piece->getColor() == enemyColor)
		{
			// 3. ダメージを適用 (Piece::takeDamage を呼び出し)
			piece->takeDamage(damage); 

			int newHealth = piece->getHealth(); 

			// 4. HPが0以下になったら削除リストに追加
			if (newHealth <= 0)
			{
				piecesToRemove.push_back(piece);
			}
		}
	}

	// 5. 削除リスト内の駒をゲーム盤から削除
	for (Piece* piece : piecesToRemove)
	{
		// 盤面上の位置を取得
		Position pos = piece->getPosition(); // ChessPiece::getPosition() を使用

		// 駒の削除処理を実行
		RemovePieceAt(pos); // SceneGameの既存の削除関数を使用

		// 盤面 (Board) からもポインタをnullにする
		board->setPieceAt(pos, nullptr);
	}
}

//マウス座標 → 盤面座標変換
Position SceneGame::ScreenToBoard(int screenX, int screenY)
{
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

// Piece を盤面座標から取得
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

void SceneGame::ApplyPersistentEffect(const ActiveEffect& effect)
{
	// 発動した効果に応じてゲーム状態を変化させる
	/*auto targetPiece = board->getPieceAt(effect.targetPos);
	 Position target = effect.targetPos;*/

	switch (effect.sourceEffectId) {
	case 3:// 悠久の盟約: 自身の駒全体の体力を2回復
	{
		const int healAmount = 2;

		// 8x8 のボード全体を走査
		for (int y = 0; y < 8; ++y) {
			for (int x = 0; x < 8; ++x) {
				Position pos = { x, y };

				// 1. Boardから論理駒 (shared_ptr<ChessPiece>) を取得
				auto logicPiece = board->getPieceAt(pos);

				// 2. 駒が存在し、効果の所有者と同じ色であるかを確認
				if (logicPiece && logicPiece->getColor() == effect.ownerColor) {

					// 3. 論理駒の体力を回復
					logicPiece->heal(healAmount);

					// ★補足: 描画駒 (Piece*) の体力ゲージの更新は、
					// 	    通常、SceneGame::Update 内で logicPiece のHPを参照して行われます。
				}
			}
		}
	}
	break;

	case 6: // 破滅の刻印: 自身を中心とする周囲8マスに2ダメージ + 無条件自爆
	{
		Position selfDestructPos = effect.targetPos;
		const int damage = 2;

		// 1. 周囲8マスにダメージを与える (周囲の死亡チェックは維持)
		for (int dy = -1; dy <= 1; ++dy) {
			for (int dx = -1; dx <= 1; ++dx) {
				Position damagePos = { selfDestructPos.x + dx, selfDestructPos.y + dy };

				flame->Play({ selfDestructPos.x * 100.0f,0.0f,selfDestructPos.y * 100.0f }, 5000.0f);
				// ボード内で、かつターゲット駒自身ではないマスをチェック
				if (board->isInsideBoard(damagePos) && (dx != 0 || dy != 0)) {
					auto victim = board->getPieceAt(damagePos);

					if (victim) {
						// ダメージ適用
						victim->takeDamage(damage);

						// 死亡チェックと削除
						if (victim->getHealth() <= 0) {
							board->setPieceAt(damagePos, nullptr);
							RemovePieceAt(damagePos);
						}
					}
				}
			}
		}

		// 2. ★無条件自爆処理: 刻印を付与されていた駒自身を削除

		// 刻印駒がその位置にいるか確認 (いなければ何もしない)
		if (board->getPieceAt(selfDestructPos)) {
			// 論理ボードから削除
			board->setPieceAt(selfDestructPos, nullptr);
			// 描画リストから削除
			RemovePieceAt(selfDestructPos);
		}

	}
	break;

	case 10: // ★移動不可解除
	{
		// ターゲット位置の駒を再度取得し直す
		// （もし駒がいない場合は何もしない）
		auto pieceToUnblock = board->getPieceAt(effect.targetPos); // 論理駒 (shared_ptr<ChessPiece>)

		if (pieceToUnblock) {
			// 駒の移動不可状態を解除
			pieceToUnblock->setImmobilized(false);
		}

		// 【重要】ここで解除されても、SceneGame::Update の駒選択時に
		//         再度 immobilize されていないか確認してください。
		break;
	}

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

bool SceneGame::IsCommonSpot(Position pos) const
{
	return pos.y >= 2 && pos.y <= 5;
}

void SceneGame::CalculateCardTargetCandidates(int effectId)
{
	cardTargetCandidates.clear();
	std::string currentTurn = board->getCurrentTurn();
	std::string enemyColor = (currentTurn == "white") ? "black" : "white";

	for (int y = 0; y < 8; ++y) {
		for (int x = 0; x < 8; ++x) {
			Position pos = { x, y };
			auto piece = board->getPieceAt(pos);

			bool isLegalTarget = false;

			// 駒がある場合のみ情報を取得する
			std::string targetColor = "";
			std::string targetType = "";
			if (piece) {
				targetColor = piece->getColor();
				targetType = piece->getType();
			}
	
			switch (effectId) {

			case 1: // 焦土の罠 (Trap): 共有マス内の【空きマス】に設置
				// piece が nullptr (空きマス) であることを条件にする
				// かつ、共通マス (y=2～5) であること
				if (!piece && pos.y >= 2 && pos.y <= 5) {
					isLegalTarget = true;
				}
				break;

			case 2: // 生命の祝福 (Buff): 自身の駒単体の体力を回復
			case 3: // 悠久の盟約 (Buff): 自身の駒全体に付与 (★トリガーとして自駒が必要)
			case 6: // 破滅の刻印 (Trap): 自身の駒単体に付与
				// piece が存在し、かつ自駒であること
				if (piece && targetColor == currentTurn) {
					isLegalTarget = true;
				}
				break;

			case 4: // 石化の鎖 (Debuff): 敵の駒に付与 (キング以外)
				// 敵駒がターゲット、かつキングではない
				if (targetColor == enemyColor && targetType != "King") {
					isLegalTarget = true;
				}
				break;
			}

			if (isLegalTarget) {
				cardTargetCandidates.push_back(pos);

			}
		}
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