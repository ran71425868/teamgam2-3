#pragma once
#include <string>

enum class CardType
{
    Buff,// 自分を強化する
    Debuff,// 敵を弱体化させる
    Trap// トラップ効果
};

struct Card
{
    std::string name;// カード名
    CardType type;// 属性（バフ、デバフ、トラップ）
    std::string effect; // 効果説明
    int effectId;// カードの効果番号

    // このカードがターン経過後に発動する持続効果を持つか
    bool isPersistent;

    // コンストラクタ（初期化を容易にするため）
    Card(std::string n, CardType t, std::string e, int id, bool persistent = false);
        
};

// CardManagerでカード使用後に外部（SceneGame）に渡す情報
struct UsedCardInfo {
    int effectId = -1;
    bool isPersistent = false;
};