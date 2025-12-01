#pragma once

#include "System/Mouse.h"
#include "System/Input.h"
#include "System/Sprite.h"
#include "Piece.h"
#include "Board.h"
#include "Card.h" // UsedCardInfoのために必要
#include <vector>
#include "CameraController.h"
#include "Scene.h"
#include "NetworkManager.h"
#include "ChessAI.h"
#include "ActiveEffectManager.h"
#include "CardManager.h"


// ゲームシーン
class SceneGame:public Scene
{
public:
	SceneGame() {};
	~SceneGame() override {};

	// 初期化
	void Initialize()override;

	// 終了化
	void Finalize()override;

	// 更新処理
	void Update(float elapsedTime)override;

	// 描画処理
	void Render() override;

	// GUI描画
	void DrawGUI()override;

private:
	Board* board = nullptr;
	Piece* piece = nullptr;

	Model* highlightModel = nullptr;
	Model* healSpotModel = nullptr;

	// ワールド座標 → スクリーン座標変換
	DirectX::XMFLOAT2 WorldToScreen(const DirectX::XMFLOAT3& worldPos) const;

	std::vector<Piece*> pieces;

	CameraController* cameraController = nullptr;


	Position selectedPos = { -1, -1 };
	std::vector<Position> legalMoves;

	// CardManagerへのポインタ
	CardManager* cardManager;

	// --- カード表示/選択システム用の追加フィールド ---

	// 描画する手札のインデックス (通常、常に手札の0番目を表示すると仮定)
	// -1 の場合、手札に表示するカードがない
	int displayHandIndex = 0;

	// 現在選択中の手札のインデックス。カード使用時に盤面ターゲットを選ぶフェーズへ移行
	int selectedHandIndex = -1;

	// カードが現在使用中（クールダウン中）であるかを示すフラグ
	bool isCardInUse = false;

	// カード使用後のクールダウン/ロック時間 (秒)
	const float CARD_COOLDOWN_TIME = 1.0f;

	// クールダウン残り時間
	float cardCooldownTimer = 0.0f;

	// カード描画位置 (画面左下を想定)
	const int CARD_DISPLAY_X = 50;
	const int CARD_DISPLAY_Y = 700;


	// 動的な回復マス生成のためのカウンター
	int blackMovedToCommonCount = 0;
	int whiteMovedToCommonCount = 0;

	// 動的に生成された回復マスが既に存在するかどうか
	bool isDynamicHealSpotActive = false;

	// 空いている共通マスをランダムに探す関数 (以前作成したもの)
	Position FindRandomEmptyCommonSpot() const;

	NetworkManager network;
	bool isServer = false;

	// ゲーム終了処理変数
	bool isGameOver = false;
	std::string winnerColor = ""; // 勝利したプレイヤーの色

	void RemovePieceAt(Position pos);

	Position ScreenToBoard(int screenX, int screenY);
	Piece* FindSlimeAt(Position pos);

	// 8x8のボードに対応する回復マス配列
	HealSpot healSpots[8][8];

	// ランダムで回復マスを生成するメソッド
	void GenerateHealSpots();

	// CardManagerのインスタンス (白と黒)
	CardManager blackCardManager;
	CardManager whiteCardManager;

	// カード使用のための状態変数
	// カードをドローする関数
	//void DrawNewCard(CardManager* cardManager);

	// 前ターンに使用されたカードの情報 (運命の反転に使用)
	UsedCardInfo lastWhiteUsedCard;
	UsedCardInfo lastBlackUsedCard;

	// ActiveEffectManagerの更新に必要な関数
	void ApplyPersistentEffect(const ActiveEffect& effect);


	// ターゲット駒が自分の駒か敵の駒かを判定するヘルパー関数
	bool IsTargetPiece(Position pos, const std::string& requiredColor) const;

	bool ApplyCardEffect(int effectId, Position targetPos);

	ChessAI* ai = nullptr;
	Sprite* check = nullptr;

	int timer = 0;
	int cooldown = 2;
};
