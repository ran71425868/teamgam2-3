#pragma once

#include "System/Sprite.h"
#include "Scene.h"

// ゲームリザルト
class SceneBlackResult :public Scene
{
public:
	SceneBlackResult() {};
	~SceneBlackResult() override {};

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
	Sprite* sprite = nullptr;
	Sprite* win = nullptr;
};