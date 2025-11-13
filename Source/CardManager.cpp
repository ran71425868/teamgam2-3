#include"CardManager.h"
#include <algorithm> // std::shuffle用
#include <random>    // 乱数生成器用

CardManager::CardManager()
{
    std::random_device rd;
    generator = std::mt19937(rd());

    // -----------------------------------------------------------------
    // デッキの初期化 (10種類 x 1枚 = 合計10枚)
    // -----------------------------------------------------------------

    // デッキの初期化 (10枚)
    // 効果ID 0〜9 (既存)
    deck = {
        {"運命の反転", CardType::Debuff, "前ターンの相手のカード効果を打ち消し", 0},
        {"焦土の罠", CardType::Trap, "指定した場所の前方2x3マスの駒に1ダメージ", 1},
        {"生命の祝福", CardType::Buff, "自身の駒単体の体力を3回復", 2},
        {"悠久の盟約", CardType::Buff, "自身の駒全体の体力を4ターン経過後に2回復", 3},
        {"石化の鎖", CardType::Debuff, "相手の駒単体の移動を制限(キングには使用不可)", 4},
        {"沈黙の呪文", CardType::Debuff, "相手のカードを1ターン使用不可", 5},
        {"破滅の刻印", CardType::Trap, "自身の駒単体に付与、3ターン後に自身を中心とする周囲8マスに2ダメージ", 6},
        {"次元の扉", CardType::Buff, "共有マス内で自身の駒単体をワープ、相手の駒取得・ワープ後の駒移動不可", 7},
        {"背水の魔弾", CardType::Debuff, "相手の駒全体に1ダメージ、カード使用後自身をターン終了", 8},
        {"叡智の探求", CardType::Buff, "カードをランダムでドロー", 9}
    };

    // デッキをシャッフル
    std::shuffle(deck.begin(), deck.end(), generator);

    // 最初の3枚を引く
    for (int i = 0; i < 3; ++i)
    {
        DrawCard();
    }
}

void CardManager::DrawCard()
{
    if (deck.empty())
    {
        // 山札が空の場合は、手札を捨ててシャッフルし、新たな山札として再利用するロジックを実装可能。
        return;
    }

    // デッキの末尾から引く (末尾がシャッフル済みなのでランダム性あり)
    Card drawn = deck.back();
    deck.pop_back();

    hand.push_back(drawn);
}

int CardManager::UseCard(int handIndex)
{
    if (isUsedCard) {
        return -1; // 使用不可
    }

    /*if (handIndex < 0 || handIndex >= hand.size()) {
        return -1;
    }*/

    Card used = hand[handIndex];

    // 手札から削除
    hand.erase(hand.begin() + handIndex);
    isUsedCard = true;

    // 使用したカードは山札に戻さない (シンプルにするため)

    return used.effectId;
}

void CardManager::StartTurn()
{
    isUsedCard = false;
    DrawCard();
}