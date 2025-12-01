#pragma once
#include "Card.h"
#include <iostream>
#include <vector>
#include <random> // std::mt19937とstd::random_deviceのために必要


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

    // 手札の枚数を取得
    size_t getHandSize() const { return hand.size(); }

    // 手札の特定のインデックスにあるカード情報を取得 (読み取り専用)
    const Card& getCardInHand(size_t index) const { return hand[index]; }

    //カードをドロー
    void DrawCard();

    // カードの効果情報（IDと持続性）を返す
    UsedCardInfo UseCard(int handIndex);

    //カードの使用状況をリセットして一枚引く(ターンの最初)
    void StartTurn();
};