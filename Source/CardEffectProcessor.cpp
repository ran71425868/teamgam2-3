// CardEffectProcessor.cpp
#include "CardEffectProcessor.h"
#include <iostream>
#include <algorithm>

// SceneGame::piecesから駒を探す (SceneGame::FindSlimeAt() に相当)
Piece* CardEffectProcessor::FindPieceAt(Position pos) {
    for (Piece* p : *pieces) {
        if (p->GetBoardPosition().x == pos.x && p->GetBoardPosition().y == pos.y) {
            return p;
        }
    }
    return nullptr;
}

// コンストラクタ
CardEffectProcessor::CardEffectProcessor(Board* b, std::vector<Piece*>* p, CardManager* wcm, CardManager* bcm,
    std::function<void(const std::string&, int)> damageCallback)
    : board(b), pieces(p), whiteCardManager(wcm), blackCardManager(bcm), ApplyDamageCallback(damageCallback) {
}

// ターゲット駒が、指定した色であるかをチェックする補助関数
bool CardEffectProcessor::IsTargetPiece(Position pos, const std::string& targetColor) {
    Piece* targetPiece = FindPieceAt(pos);
    return targetPiece && targetPiece->getColor() == targetColor;
}

// ターゲット選択を伴うカード効果の適用 (ID 4, 6)
bool CardEffectProcessor::ApplyTargetedEffect(int effectId, Position targetPos, const std::string& currentTurn) {
    std::string enemyColor = (currentTurn == "white") ? "black" : "white";
    Piece* targetPiece = FindPieceAt(targetPos);

    switch (effectId) {

        // 焦土の罠 (Trap): 指定した場所の前方 2x3 マスの駒に 1 ダメージ
    case 1:
    {
        // ターゲット位置がボード内か確認
        if (!board->isInsideBoard(targetPos)) {
            return false; // 無効な位置
        }

        // 1. ダメージエリアの定義
        int damage = 1;

        // 進行方向の定義 (白: y軸減少方向へ前方, 黒: y軸増加方向へ前方)
        // 白 (y=7, 6 から y=0, 1 へ移動) の場合、前方マスは y - 1, y - 2
        // 黒 (y=0, 1 から y=7, 6 へ移動) の場合、前方マスは y + 1, y + 2
        const int direction = (currentTurn == "white") ? -1 : 1;

        // 展開エリア: 前方2マス (y+direction*1, y+direction*2)、横幅3マス (x-1, x, x+1)
        // 前方 2 マス (dy = 1, 2)
        for (int dy = 1; dy <= 2; ++dy) {
            // 横幅 3 マス (dx = -1, 0, 1)
            for (int dx = -1; dx <= 1; ++dx) {

                // ダメージを与えるマスの座標を計算
                Position damagePos = {
                    targetPos.x + dx,
                    targetPos.y + (direction * dy) // 進行方向へ展開
                };

                // 2. ボード内チェックとダメージ適用
                if (board->isInsideBoard(damagePos)) {
                    auto victim = board->getPieceAt(damagePos);

                    if (victim) {
                        // 駒が存在する場合、ダメージを与える
                        victim->takeDamage(damage);

                        // 死亡チェック (takeDamage後に health <= 0 になったら、リストから削除する処理が必要)
                        if (victim->getHealth() <= 0) {
                            //board->RemovePieceAt(victim);
                        }
                    }
                }
            }
        }
        // トラップ設置は成功したとみなす
        return true;

    }
        		
    case 2: // 生命の祝福 (Buff): 自身の駒単体の体力を3回復
    {
        const int healAmount = 3;

        // 1. ターゲット駒が存在し、かつそれが自駒であるかを確認
        if (targetPiece && IsTargetPiece(targetPos, currentTurn)) {

            // 2. 体力を回復
            targetPiece->heal(healAmount);


            return true; // 効果適用成功
        }

        // ターゲットが無効（駒がいない、または敵駒）
        return false;
    }

    // 悠久の盟約 (Buff): 自身の駒全体に付与、4ターン後に体力を2回復
    case 3: 
    {
        // 1. ターゲット駒が存在し、かつそれが自駒であるかを確認 (カード発動のトリガーとして使用)
        if (targetPiece && IsTargetPiece(targetPos, currentTurn)) {

            // 2. 持続効果データを作成 (ID 3, 4ターン後に発動)
            // 全体効果のため、targetPos は無効な位置を登録
            ActiveEffect effect(
                3, // 効果ID
                4, // 残りターン数
                { -1, -1 }, // ★修正: 全体効果を示すため、無効な位置をターゲットとして登録
                currentTurn // 所有者
            );

            // 3. ActiveEffectManagerに登録
            ActiveEffectManager::GetInstance().AddEffect(effect);

            return true; // 効果適用成功
        }
        return false; // ターゲットが無効（駒がいない、または敵駒）
    }

    // case 4: 石化の鎖 (Debuff)
    case 4:
    {
        if (targetPiece && IsTargetPiece(targetPos, enemyColor)) {
            if (targetPiece->getType() == "King") {
                
                return false;
            }
            targetPiece->setImmobilized(true);
           
            return true;
        }
        return false;
    }

    // case 6: 破滅の刻印 (Trap)
    case 6:
    {
        if (targetPiece && IsTargetPiece(targetPos, currentTurn)) {
            ActiveEffect effect(6, 3, targetPos, currentTurn);
            ActiveEffectManager::GetInstance().AddEffect(effect);
            return true;
        }
        return false;
    }

    // ... その他のターゲット選択が必要なカードロジック (ID 1, 2, 7など) もここに追加
    



    default:
        return false;
    }
}

// 即時発動でターゲット選択が不要なカード効果処理 (ID 0, 8, 9)
void CardEffectProcessor::ProcessInstantCard(int effectId, const std::string& currentTurn, CardManager* cardManager,
    int selectedHandIndex, bool& isCardInUse, float& cardCooldownTimer, const float CARD_COOLDOWN_TIME) {

    std::string enemyColor = (currentTurn == "white") ? "black" : "white";
    bool cardUsed = false;

    switch (effectId) {
    case 0: // 運命の反転: 相手の持続効果を打ち消し
    {
        int count3 = ActiveEffectManager::GetInstance().CancelEffect(3, enemyColor); // 悠久の盟約
        int count6 = ActiveEffectManager::GetInstance().CancelEffect(6, enemyColor); // 破滅の刻印

        cardUsed = true;
        break;
    }

    case 5: // 沈黙の呪文: 相手のカードを1ターン使用不可 (★ユーザーの指示に基づき追加)
    {
        // 相手の CardManager を取得 (currentTurn が white なら blackCardManager を、black なら whiteCardManager を取得)
        CardManager* enemyCardManager = (currentTurn == "white") ? blackCardManager : whiteCardManager;

        // ★重要: 相手のカード使用を禁止するフラグを設定するロジックが必要です。
        // このフラグは、SceneGame.h/cpp または CardManager.h/cpp に追加し、
        // 相手のターン開始時にカード使用チェックで使用される必要があります。

        // 例として、CardManagerに setCardUseDisabled(bool) 関数があると仮定した場合の処理
        // enemyCardManager->setCardUseDisabled(true); 

        cardUsed = true;
        break;
    }

    case 8: // 絶対の雷: 相手の駒全体に1ダメージ
    {
        // SceneGame::ApplyDamageToAllEnemyPieces をコールバックで実行
        if (ApplyDamageCallback) {
            ApplyDamageCallback(enemyColor, 1);
            cardUsed = true;
        }
        // ★ターン終了処理は SceneGame::Update 側で行う
        break;
    }

    case 9: // 叡智の探求: カードをランダムでドロー
    {
        cardManager->DrawCard();
        cardUsed = true;
        break;
    }

    default:
        return; // 処理なし
    }

    if (cardUsed) {
        // 成功時共通処理
        isCardInUse = true;
        cardCooldownTimer = CARD_COOLDOWN_TIME;
        cardManager->UseCard(selectedHandIndex);
    }
}