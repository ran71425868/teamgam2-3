#include "EnemyManager.h"
#include "Collision.h"


//エネミー削除
void EnemyManager::Remove(Enemy* enemy)
{
	//破棄リストに追加
	removes.insert(enemy);

}
//更新処理
void EnemyManager::Update(float elapsedTime)
{
	for (Enemy* enemy : enemies)
	{
		enemy->Update(elapsedTime);
	}

	//破棄処理
	//*enemiesの範囲for文中でerase()すると不具合が発生してしまうため、
	//更新処理が終わった後に破棄リストに積まれたオブジェクトを削除する。
	for (Enemy* enemy : removes)
	{
		//std::vectorから要素を削除する場合はイテレーターで削除しなければならない
		std::vector<Enemy*>::iterator it = std::find(
			enemies.begin(), //ここから
			enemies.end(), //ここまでの中から
			enemy//こいつがあるかどうかチェック
		);

		if (it != enemies.end())
		{
			enemies.erase(it);
		}

		//弾丸の破棄処理
		delete enemy;
	}
	//破棄リストをクリア
	removes.clear();

	//敵同士の衝突処理
	CollisionEnemyVsEnemies();
}

//描画処理
void EnemyManager::Render(const RenderContext& rc, ModelRenderer* renderer)
{
	for (Enemy* enemy : enemies)
	{
		enemy->Render(rc, renderer);
	}
}

//エネミー登録
void EnemyManager::Register(Enemy* enemy)
{
	enemies.emplace_back(enemy);
}


//弾丸全削除
void EnemyManager::Clear()
{
	for (Enemy* enemy : enemies)
	{
		delete enemy;
	}
	enemies.clear();
}

//デバッグプリミティブ描画
void EnemyManager::RenderDebugPrimitive(const RenderContext& rc, ShapeRenderer* renderer)
{
	for (Enemy* enemy : enemies)
	{
		enemy->RenderDebugPrimitive(rc, renderer);
	}
}

//エネミー同士の衝突処理
void EnemyManager::CollisionEnemyVsEnemies()
{
	EnemyManager& enemyManager = EnemyManager::Instance();

	//全ての敵と総当たりで衝突処理
	int enemyCount = enemyManager.GetEnemyCount();

	for (int i = 0; i < enemyCount; i++)
	{
		for (int j = i + 1; j < enemyCount; j++)
		{
			Enemy* enemy1 = enemyManager.GetEnemy(i);
			Enemy* enemy2 = enemyManager.GetEnemy(j);

			//衝突処理
			DirectX::XMFLOAT3 outPosition;
			//if (Collision::IntersectSphereVsSphere(enemy1->GetPosition(),enemy1->GetRadius(), enemy2->GetPosition(), enemy2->GetRadius(), outPosition))
			//{
			//	//押し出し後の位置設定
			//	enemy2->SetPosition(outPosition);
			//	
			//}

			if (Collision::IntersectCylinderVsCylinder(
				enemy1->GetPosition(),
				enemy1->GetRadius(), 
				enemy1->GetHeight(), 
				enemy2->GetPosition(),
				enemy2->GetRadius(),
				enemy2->GetHeight(),
				outPosition))
			{
				//押し出し後の位置設定
				enemy2->SetPosition(outPosition);
			}
		}
	}

}
