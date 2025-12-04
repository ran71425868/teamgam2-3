// CardEffectProcessor.h
#pragma once

#include <functional> // std::function のために必要
#include <vector>
#include <string>
#include "Board.h" 
#include "Piece.h" 
#include "CardManager.h" 
#include "ActiveEffectManager.h" 

class CardEffectProcessor {
private:
    // ゲームの状態へのポインタ
    Board* board;
    std::vector<Piece*>* pieces; // SceneGame::pieces へのポインタ
    CardManager* whiteCardManager;
    CardManager* blackCardManager;

    // ID 8 (絶対の雷) のためのコールバック関数。SceneGame::ApplyDamageToAllEnemyPieces() を実行
    std::function<void(const std::string&, int)> ApplyDamageCallback;

    // 補助関数: 駒の検索 (SceneGame::FindSlimeAt() と同様のロジック)
    Piece* FindPieceAt(Position pos);

public:
    // コンストラクタ
    CardEffectProcessor(Board* b, std::vector<Piece*>* p, CardManager* wcm, CardManager* bcm,
        std::function<void(const std::string&, int)> damageCallback);

    // 補助関数: ターゲットが指定した色か確認
    bool IsTargetPiece(Position pos, const std::string& targetColor);

    /**
     * @brief ターゲット選択を伴うカード効果の適用 (ID 4, 6, ...)
     * @return 成功した場合 true
     */
    bool ApplyTargetedEffect(int effectId, Position targetPos, const std::string& currentTurn);

    /**
     * @brief 即時発動でターゲット選択が不要なカード効果処理 (ID 0, 8, 9)
     * @param isCardInUse, cardCooldownTimer SceneGameの状態変数の参照
     */
    void ProcessInstantCard(int effectId, const std::string& currentTurn, CardManager* cardManager,
        int selectedHandIndex, bool& isCardInUse, float& cardCooldownTimer, const float CARD_COOLDOWN_TIME);
};