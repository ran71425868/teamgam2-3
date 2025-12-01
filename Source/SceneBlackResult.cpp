#include "System/Graphics.h"
#include "SceneTitle.h"
#include "SceneBlackResult.h"
#include "System/Input.h"
#include "SceneManager.h"
#include "SceneGame.h"

//初期化
void SceneBlackResult::Initialize()
{
	sprite = new Sprite("Data/Sprite/仮リザルト.png");
	win = new Sprite("Data/Sprite/black_win.png");
}

//終了化
void SceneBlackResult::Finalize()
{
	if (sprite != nullptr)
	{
		delete sprite;
		sprite = nullptr;
	}
}

//更新処理
void SceneBlackResult::Update(float elapsedTime)
{
	GamePad& gamePad = Input::Instance().GetGamePad();

	//なにかボタンを押したらゲームシーンへ切り替え
	const GamePadButton anyButton =
		GamePad::BTN_A
		| GamePad::BTN_B
		| GamePad::BTN_X
		| GamePad::BTN_Y
		;

	if (gamePad.GetButtonDown() & anyButton)
	{
		SceneManager::Instance().ChangeScene(new SceneTitle);
	}
}

//描画処理
void SceneBlackResult::Render()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	RenderState* renderState = graphics.GetRenderState();

	// 描画準備
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = graphics.GetRenderState();

	// 2Dスプライト描画
	{
		//タイトル画面
		float screenWidth = static_cast<float>(graphics.GetScreenWidth());
		float screenHeight = static_cast<float>(graphics.GetScreenHeight());
		sprite->Render(rc,
			0, 0, 0, screenWidth, screenHeight,
			0,
			1, 1, 1, 1);

		win->Render(rc,
			550, 0, 0, 800, 500,
			0,
			1, 1, 1, 1);
	}

}

//GUI描画
void SceneBlackResult::DrawGUI()
{
}
