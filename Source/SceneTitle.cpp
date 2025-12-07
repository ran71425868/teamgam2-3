#include "System/Graphics.h"
#include "SceneTitle.h"
#include "System/Input.h"
#include "SceneGame.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "SceneSelect.h"
#include "System/Audio.h"

//初期化
void SceneTitle::Initialize()
{
	sprite = new Sprite("Data/Sprite/タイトル.png");

	BGM = Audio::Instance().LoadAudioSource("Data/Sound/Title.wav");

	BGM->Play(false);
	BGM->SetVolume(0.4f);
}

//終了化
void SceneTitle::Finalize()
{
	if (sprite != nullptr)
	{
		delete sprite;
		sprite = nullptr;
	}
	delete BGM;
}

//更新処理
void SceneTitle::Update(float elapsedTime)
{
	Mouse& mouseCursor = Input::Instance().GetMouse();

	const MouseButton anyButton =
		Mouse::BTN_LEFT;
	GamePad& gamePad = Input::Instance().GetGamePad();

	//なにかボタンを押したらゲームシーンへ切り替え
	const GamePadButton anykeyButton =
		  GamePad::BTN_A
		| GamePad::BTN_B
		| GamePad::BTN_X
		| GamePad::BTN_Y
		;

	if (gamePad.GetButtonDown() & anykeyButton)
	{
		BGM->Stop();
		SceneManager::Instance().ChangeScene(new SceneLoading(new SceneSelect));
	}

	if (mouseCursor.GetButtonDown() & anyButton)
	{
		BGM->Stop();
		SceneManager::Instance().ChangeScene(new SceneLoading(new SceneSelect));
	}
}

//描画処理
void SceneTitle::Render()
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
	}

}

//GUI描画
void SceneTitle::DrawGUI()
{
}
