#pragma once


#include <string>
#include <iostream>

// チェス盤上の位置を示す構造体（SceneGameの定義を仮定）
struct Position {
    int x;
    int y;
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

/**
 * @brief ターン経過後に発動する持続効果のデータ構造
 */
struct ActiveEffect
{
    // この効果が生まれた元となったカードのID
    int sourceEffectId;

    // 効果が発動するまでの残りターン数 (例: 3ターン後に発動なら 3, 2, 1, 0 で発動)
    int remainingTurns;

    // この効果がどの駒に付与されたか、またはどの座標に設置されたか
    Position targetPos;

    // この効果の所有者（どのプレイヤーが設置したか）
    std::string ownerColor;

    // 持続効果の種類を区別するためのID（必要に応じて）
    // 例: 3 (悠久の盟約 - 継続バフ), 6 (破滅の刻印 - トラップ)

    /**
     * @brief 持続効果のコンストラクタ
     * @param id 元のカード効果ID
     * @param turns 残りターン数
     * @param pos ターゲット位置
     * @param color 効果の所有者
     */
    ActiveEffect(int id, int turns, Position pos, std::string color)
        : sourceEffectId(id), remainingTurns(turns), targetPos(pos), ownerColor(color) {
    }
};