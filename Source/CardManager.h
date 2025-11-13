#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <random> // std::mt19937とstd::random_deviceのために必要


enum class CardType
{
    Buff, // 自分を強化する
    Debuff,// 敵を弱体化させる
    Trap // トラップ効果
};

struct Card
{
    std::string name;// カード名
    CardType type;// 属性（バフ、デバフ、トラップ）
    std::string effect; // 効果説明
    int effectId;// カードの効果番号
};

class CardManager
{
private:
    std::vector<Card> deck;// 山札（全カード12枚）
    std::vector<Card> hand;// 手札
    bool isUsedCard;// このターンカードを使ったか
    std::mt19937 generator; // 乱数生成器の宣言

public:

    //手札を参照
    const std::vector<Card>& GetHand() const
    {
        return hand;
    }

    //コンストラクタ
    CardManager();

    //カードをドロー
    void DrawCard();

    //カードの効果番号を返す
    int UseCard(int handIndex);

    //カードの使用状況をリセットして一枚引く(ターンの最初)
    void StartTurn();
};