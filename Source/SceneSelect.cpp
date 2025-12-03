#include "System/Graphics.h"
#include "SceneSelect.h"
#include "System/Input.h"
#include "SceneGame.h"
#include "SceneMultiGame.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "SceneTutorial.h"
//初期化
void SceneSelect::Initialize()
{

	sprite = new Sprite("Data/Sprite/仮セレクト.png");
	sprite2 = new Sprite("Data/Sprite/チュートリアル.png");
	sprite3 = new Sprite("Data/Sprite/シングル.png");
	sprite4 = new Sprite("Data/Sprite/マルチ.png");

}

//終了化
void SceneSelect::Finalize()
{
	if (sprite != nullptr)
	{
		delete sprite;
		sprite = nullptr;
	}
	/*if (sprite2 != nullptr)
	{
		delete sprite2;
		sprite2 = nullptr;
	}
	if (sprite3 != nullptr)
	{
		delete sprite3;
		sprite3 = nullptr;
	}
	if (sprite4 != nullptr)
	{
		delete sprite4;
		sprite4 = nullptr;
	}*/
}

//更新処理
void SceneSelect::Update(float elapsedTime)
{
	Mouse& mouseCursor = Input::Instance().GetMouse();

	const MouseButton anyButton =
		Mouse::BTN_LEFT;

	//長方形の描画位置
	DirectX::XMFLOAT2 tutorialleft = { 200,450 };
	DirectX::XMFLOAT2 tutorialright = { 400,600 };

	DirectX::XMFLOAT2 stageleft = { 800,450 };
	DirectX::XMFLOAT2 stageright = { 1000,600 };

	DirectX::XMFLOAT2 stage2left = { 1400,400 };
	DirectX::XMFLOAT2 stage2right = { 1600,550 };


	//ボタンの描画をしているとこに当たり判定
	if (tutorialleft.x < mouseCursor.GetPositionX() && tutorialright.x > mouseCursor.GetPositionX() && tutorialleft.y<mouseCursor.GetPositionY() && tutorialright.y>mouseCursor.GetPositionY())
	{
		if (mouseCursor.GetButtonDown() & anyButton)
		{
			SceneManager::Instance().ChangeScene(new SceneLoading(new SceneTutorial));
		}
	}

	if (stageleft.x < mouseCursor.GetPositionX() && stageright.x > mouseCursor.GetPositionX() && stageleft.y<mouseCursor.GetPositionY() && stageright.y>mouseCursor.GetPositionY())
	{
		if (mouseCursor.GetButtonDown() & anyButton)
		{
			SceneManager::Instance().ChangeScene(new SceneLoading(new SceneGame));
		}
	}

	if (stage2left.x < mouseCursor.GetPositionX() && stage2right.x > mouseCursor.GetPositionX() && stage2left.y<mouseCursor.GetPositionY() && stage2right.y>mouseCursor.GetPositionY())
	{
		if (mouseCursor.GetButtonDown() & anyButton)
		{
			SceneManager::Instance().ChangeScene(new SceneLoading(new SceneMultiGame));
		}
	}


}

//描画処理
void SceneSelect::Render()
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
			0, 0, 0, 1920, 1080,
			0,
			1, 1, 1, 1);

		sprite2->Render(rc,
			120, 400, 0, 400, 300,
			0,
			1, 1, 1, 1);

		sprite3->Render(rc,
			700, 400, 0, 500, 300,
			0,
			1, 1, 1, 1);

		sprite4->Render(rc,
			1300, 300, 0, 500, 500,
			0,
			1, 1, 1, 1);

	}

}

//GUI描画
void SceneSelect::DrawGUI()
{
}
