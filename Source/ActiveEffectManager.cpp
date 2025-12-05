#include "ActiveEffectManager.h"
#include <iostream>
#include <algorithm>

void ActiveEffectManager::AddEffect(const ActiveEffect& effect)
{
    activeEffects.push_back(effect);
    std::cout << "[Effect Manager] 新しい持続効果を追加しました (ID: " << effect.sourceEffectId
        << ", ターゲット: (" << effect.targetPos.x << ", " << effect.targetPos.y << "))" << std::endl;
}

// ターン更新のロジック
int ActiveEffectManager::UpdateTurn(void* gameContext)
{
    int activatedCount = 0;

    // 発動した効果、または期限切れの効果を保持するリスト
    std::vector<ActiveEffect> effectsToProcess;

    // 1. 全てのアクティブな効果をチェック
    for (auto& effect : activeEffects)
    {
        effect.remainingTurns--; // ターンを減らす

        // 2. 発動条件のチェック (残りターンが0以下になったら発動)
        if (effect.remainingTurns <= 0)
        {
            effectsToProcess.push_back(effect);
            activatedCount++;
        }
    }

    // 3. 発動した効果をリストから削除
    // remainingTurns <= 0 のものを削除
    activeEffects.erase(std::remove_if(activeEffects.begin(), activeEffects.end(),
        [](const ActiveEffect& e) { return e.remainingTurns <= 0; }), activeEffects.end());

    // 4. 発動した効果を処理（ここではログ出力のみ）
    for (const auto& effect : effectsToProcess)
    {
        std::cout << "[Effect Activated] ID: " << effect.sourceEffectId
            << " (所有者: " << effect.ownerColor << ") がターゲット ("
            << effect.targetPos.x << ", " << effect.targetPos.y << ") で発動しました。" << std::endl;

        // ★重要: ここで SceneGame の関数を呼び出し、実際のゲームロジック（体力回復/ダメージなど）を適用します。
        // 例: SceneGame* game = static_cast<SceneGame*>(gameContext);
        //     game->ApplyPersistentEffect(effect);
    }

    return activatedCount;
}

// 打ち消しロジック（運命の反転）
int ActiveEffectManager::CancelEffect(int targetEffectId, const std::string& enemyColor)
{
    int cancelledCount = 0;

    // ターゲットとなる持続効果（相手の色によって発動されたもの）をリストから削除
    // この関数は、効果ID 0 が使用されたときに、SceneGame から呼び出されます。

    auto it = activeEffects.begin();
    while (it != activeEffects.end())
    {
        // 1. 打ち消し対象のカードIDであること
        // 2. 相手 (enemyColor) によって設置された効果であること
        if (it->sourceEffectId == targetEffectId && it->ownerColor == enemyColor)
        {
            std::cout << "[Effect Cancelled] 効果ID " << it->sourceEffectId
                << " (ターゲット: (" << it->targetPos.x << ", " << it->targetPos.y << ")) を打ち消しました。" << std::endl;

            it = activeEffects.erase(it); // 効果を削除
            cancelledCount++;
        }
        else
        {
            ++it;
        }
    }

    return cancelledCount;
}
// ★追加: ターン経過時の効果処理を実行する関数
void ActiveEffectManager::ProcessTurnEffects(const std::string& currentTurn, EffectCallback effectCallback)
{
    std::vector<ActiveEffect> effectsToKeep;

    // 1. 全ての持続効果をチェック
    for (auto& effect : activeEffects) {

        // ターン切り替えで効果を発動するのは、その効果の所有者ではないプレイヤーのターンが始まった時
        // 例: 白が仕掛けた効果は、黒のターン開始時に発動する（ターンを消費する）。

        // ここでは、効果の所有者が現在ターンではない場合にのみ、ターン数を減らすと仮定します。
        // もし「カードを使ったプレイヤーのターン終了時にカウントが進む」仕様であれば、
        // この if 文の条件を見直してください。

        // 【一般的な仕様に合わせた処理】: ターンが切り替わった時点で、
        // 全ての効果の残りターン数を減らす

        effect.remainingTurns--; // 残りターン数を減らす

        // 2. ターン数が 0 になった効果を処理し、コールバックで SceneGame に渡す
        if (effect.remainingTurns <= 0) {

            // 効果を発動
            effectCallback(effect);

            // remainingTurns が 0 以下になった効果はリストから除外されるため、effectsToKeepには追加しない
        }
        else {
            // まだターンが残っている効果はリストに残す
            effectsToKeep.push_back(effect);
        }
    }

    // 3. リストを更新 (ターンが残っている効果のみを残す)
    activeEffects = std::move(effectsToKeep);
}
