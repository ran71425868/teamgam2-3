#pragma once

#include "System/Mouse.h"
#include "System/Input.h"
#include "Stage.h"
#include "Slime.h"
#include "Board.h"
#include <vector>
#include "CameraController.h"
#include "Scene.h"
#include "NetworkManager.h"

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
	Slime* slime = nullptr;

	Model* highlightModel = nullptr;

	std::vector<Slime*> pieces;
	//Player* player = nullptr;
	CameraController* cameraController = nullptr;


	Position selectedPos = { -1, -1 };
	std::vector<Position> legalMoves;

	NetworkManager network;
	bool isServer = false;

	// ゲーム終了処理変数
	bool isGameOver = false;
	std::string winnerColor = ""; // 勝利したプレイヤーの色

	void RemoveSlimeAt(Position pos);

	Position ScreenToBoard(int screenX, int screenY);
	Slime* FindSlimeAt(Position pos);


};
