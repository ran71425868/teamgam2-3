#pragma once

#include "System/Mouse.h"
#include "System/Input.h"
#include "Stage.h"
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
	Stage* stage = nullptr;
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
	// ...

	// 前ターンに使用されたカードの情報 (運命の反転に使用)
	UsedCardInfo lastWhiteUsedCard;
	UsedCardInfo lastBlackUsedCard;

	// ActiveEffectManagerの更新に必要な関数
	void ApplyPersistentEffect(const ActiveEffect& effect);


	// ターゲット駒が自分の駒か敵の駒かを判定するヘルパー関数
	bool IsTargetPiece(Position pos, const std::string& requiredColor) const;

	bool ApplyCardEffect(int effectId, Position targetPos);

	ChessAI* ai = nullptr;
};
