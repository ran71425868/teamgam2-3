#pragma once

#include "System/Mouse.h"
#include "System/Input.h"
#include "System/Sprite.h"
#include "System/AudioSource.h"
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
#include <memory> // std::unique_ptr のために追加
#include "CardEffectProcessor.h" // ★追加

enum class CardEffectState
{
	NONE,                   // カード効果処理なし
	DIMENSIONAL_GATE_SELECT_PIECE, // 次元の扉: ワープ元駒の選択待ち
	DIMENSIONAL_GATE_SELECT_TARGET, // 次元の扉: ワープ先マスの選択待ち

	// ... 他のカード効果のフェーズ ...
	// 駒をターゲットとして選択するのを待機する状態
	AWAITING_PIECE_SELECTION,
	// 敵駒/自駒を問わず、ターゲット駒1つを選択する待ち状態
	AWAITING_SINGLE_PIECE_TARGET,
	// 盤面上のマスをターゲットとして選択するのを待機する状態
	AWAITING_SQUARE_TARGET,
};

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

	CardManager* cardManager; // CardManagerへのポインタ (Initializeで設定が必要)

	// CardManagerのインスタンス (白と黒)
	CardManager blackCardManager;
	CardManager whiteCardManager;

	std::vector<Position> dimensionalGateTargets; // ワープ可能な空きマスのリスト


	// カード効果処理を担うインスタンス
	std::unique_ptr<CardEffectProcessor> cardProcessor;

	// --- カード表示/選択システム用の追加フィールド ---

	// 現在選択中の手札のインデックス (0, 1, 2)。-1 は未選択を意味するが、
	// ドローされている場合は常にどれかを選択状態にするのが自然なため、0〜2の範囲
	int selectedHandIndex = 0;

	// カードが使用中（クールダウン中）であるかを示すフラグ (前の回答から流用)
	bool isCardInUse = false;

	// カード使用後のクールダウン/ロック時間 (秒) (前の回答から流用)
	const float CARD_COOLDOWN_TIME = 1.0f;

	// クールダウン残り時間 (前の回答から流用)
	float cardCooldownTimer = 0.0f;

	// カード描画開始位置 (画面左下を想定)
	const int CARD_START_X = 50;
	const int CARD_START_Y = 750;
	const int CARD_WIDTH = 100;   // 例: 816 / 10 = 81.6 (手札の幅)
	const int CARD_HEIGHT = 160; // 例: 1456 / 10 = 145.6 (手札の高さ)
	const int CARD_SPACING = 15; // カード間の間隔

	// カード効果IDごとのテクスチャを保持する配列(0〜9の10枚)
	Sprite * cardSprites[10] = { nullptr };

	//  カード説明用のスプライト配列
	Sprite* cardDescriptionSprites[10];
	
	// 前ターンに使用されたカードの情報 (運命の反転に使用)
	UsedCardInfo lastWhiteUsedCard;
	UsedCardInfo lastBlackUsedCard;

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

	// 特定の色のすべての駒にダメージを与える
	void ApplyDamageToAllEnemyPieces(const std::string& enemyColor, int damage);

	Position ScreenToBoard(int screenX, int screenY);
	Piece* FindSlimeAt(Position pos);

	// 8x8のボードに対応する回復マス配列
	HealSpot healSpots[8][8];

	// ランダムで回復マスを生成するメソッド
	void GenerateHealSpots();

	// カード使用のための状態変数
	CardEffectState currentCardEffectState = CardEffectState::NONE;
	int selectedCardEffectId = -1; // 現在処理中のカードの効果ID
	Piece* selectedPieceForEffect = nullptr; // カード効果の対象として選択された駒

	// カード効果のターゲット候補の座標リスト
	std::vector<Position> cardTargetCandidates; // ← これを追加

	// ActiveEffectManagerの更新に必要な関数
	void ApplyPersistentEffect(const ActiveEffect& effect);

	// ターゲット駒が自分の駒か敵の駒かを判定するヘルパー関数
	bool IsTargetPiece(Position pos, const std::string& requiredColor) const;

	/**
	* @brief 指定された位置が共通マス (y=2 から y=5) であるかを判定
	*/
	bool IsCommonSpot(Position pos) const;

	// カードのターゲット候補を計算するヘルパー関数
	void CalculateCardTargetCandidates(int effectId);

	ChessAI* ai = nullptr;
	Sprite* check = nullptr;
	// ★追加: 前のターンがどちらの色だったかを記憶する
	std::string lastTurnColor = "white";

	AudioSource* BGM = nullptr;
	AudioSource* ATKSE = nullptr;
	AudioSource* DEFSE = nullptr;
	AudioSource* MOVESE = nullptr;
	AudioSource* Unauthorized_Click = nullptr;
	AudioSource* Click = nullptr;

	AudioSource* WHOLEHEALSE = nullptr;
	AudioSource* BOMB2 = nullptr;

	int timer = 0;
	int cooldown = 2;
	int endtimer = 0.0f;

	Effect* deleteEffect = nullptr;
	Effect* flame = nullptr;
	Effect* Heal = nullptr;
};
