#pragma once

#include "ActiveEffect.h"
#include <vector>
#include <string>
#include <memory> // std::unique_ptr用

/**
 * @brief ゲーム内の持続的なカード効果を一元管理するシングルトンクラス
 * * ターン毎の更新、効果の発動チェック、打ち消し処理を担当します。
 */
class ActiveEffectManager
{
private:
    std::vector<ActiveEffect> activeEffects;

    // シングルトンパターン: コンストラクタを private に
    ActiveEffectManager() = default;
    ~ActiveEffectManager() = default;

    // コピーと代入を禁止
    ActiveEffectManager(const ActiveEffectManager&) = delete;
    ActiveEffectManager& operator=(const ActiveEffectManager&) = delete;

public:
    /**
     * @brief シングルトンのインスタンスを取得
     */
    static ActiveEffectManager& GetInstance()
    {
        static ActiveEffectManager instance;
        return instance;
    }

    /**
     * @brief 新しい持続効果を追加
     * @param effect 追加する効果のデータ
     */
    void AddEffect(const ActiveEffect& effect);

    /**
     * @brief ターン終了時に呼ばれ、タイマーを更新し、発動する効果を処理
     * @param gameContext SceneGameへのポインタやインターフェースなど、効果適用に必要な情報
     * @return 発動した効果の数
     */
    int UpdateTurn(void* gameContext);

    /**
     * @brief 特定のカード効果を全て打ち消す（運命の反転の実装）
     * @param targetEffectId 打ち消す効果のID (例: 3または6)
     * @param enemyColor 打ち消しのターゲットとなるプレイヤーの色 (この色によって発動した効果のみが対象)
     * @return 打ち消した効果の数
     */
    int CancelEffect(int targetEffectId, const std::string& enemyColor);
};