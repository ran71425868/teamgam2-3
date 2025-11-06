#pragma once

#include "System/Mouse.h"
#include "System/Input.h"
#include "Stage.h"
//#include "Player.h"
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

	std::vector<Slime*> pieces;
	//Player* player = nullptr;
	CameraController* cameraController = nullptr;

	NetworkManager network;
	bool isServer = false;


	Position selectedPos = { -1, -1 };
	std::vector<Position> legalMoves;

	Position ScreenToBoard(int screenX, int screenY);
	Slime* FindSlimeAt(Position pos);


};
