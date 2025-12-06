// CardEffectProcessor.cpp
#include "CardEffectProcessor.h"
#include <iostream>
#include "ActiveEffectManager.h"
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
/**
 * @brief 指定位置の Piece* オブジェクトを pieces リストから削除します。
 * @param pos 削除対象の駒のボード座標
 */
void CardEffectProcessor::RemovePieceFromList(Position pos) {
    auto& pieces_ref = *pieces; // std::vector<Piece*>* から参照を取得

    auto it = std::remove_if(pieces_ref.begin(), pieces_ref.end(),
        [pos](Piece* p) {
            if (p) {
                return p->GetBoardPosition().x == pos.x && p->GetBoardPosition().y == pos.y;
            }
            return false;
        });

    // 削除された要素を実際にリストから除去
    pieces_ref.erase(it, pieces_ref.end());
}
// コンストラクタ
CardEffectProcessor::CardEffectProcessor(Board* b, std::vector<Piece*>* p, CardManager* wcm, CardManager* bcm,
    std::function<void(const std::string&, int)> damageCallback)
    : board(b), pieces(p), whiteCardManager(wcm), blackCardManager(bcm), ApplyDamageCallback(damageCallback)
{
    magic = new Effect("Data/Effect/magic.efk");
    heal = new Effect("Data/Effect/heal.efk");
    thunder= new Effect("Data/Effect/thunder.efk");
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

                        // 死亡チェック
                        if (victim->getHealth() <= 0) {
                            // 論理ボードから削除
                            board->setPieceAt(damagePos, nullptr);
                            // 描画リストから削除 (★追加)
                            RemovePieceFromList(damagePos);
                        }
                    }
                }
            }
        }
        // トラップ設置は成功したとみなす
        return true;

    }
        		
    // 生命の祝福 (Buff): 自身の駒単体の体力を3回復
    case 2:
    {
        const int healAmount = 3;

        // 1. BoardからChessPiece (スマートポインタ) を取得
        std::shared_ptr<ChessPiece> sharedPiece = board->getPieceAt(targetPos);

        // 2. 駒が存在し、かつそれが現在のターンプレイヤーの駒であるかを確認
        // sharedPiece->get() で生のポインタ (ChessPiece*) を取得し、NULLチェックを兼ねる
        if (sharedPiece && sharedPiece->getColor() == currentTurn) {

            // 3. 論理オブジェクト (ChessPiece) の体力を回復
            sharedPiece->heal(healAmount);
            heal->Play({ targetPos.x * 100.0f,10.0f,targetPos.y * 100.0f }, 50);
            // 4. 描画オブジェクト (Piece) の更新
            // Pieceオブジェクトは通常、SceneGame::Updateで体力表示を更新しますが、
            // 即時反映を確実にするため、ここで Piece* を取得して更新処理を呼び出す必要があるかもしれません。
            // (SceneGame::FindSlimeAt()のようなヘルパー関数が必要)

            return true; // 効果適用成功
        }

        // ターゲットが無効（駒がいない、または敵駒）
        return false;
    }

    // 悠久の盟約 (Buff): 自身の駒全体に付与、4ターン後に体力を2回復
    case 3: 
    {
        // 1. ターゲット駒が存在し、かつそれが自駒であるかを確認 (カード発動のトリガーとして使用)

       heal->Play({ 3.5 * 100.0f,10.0f,3.5 * 100.0f }, 200);
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
        thunder->Play({3.5 * 100.0f, 10.0f, 3.5 * 100.0f }, 150);
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

// =================================================================
// ★ 追加: 次元への扉 (ID 7) の効果を適用
// =================================================================
bool CardEffectProcessor::ApplyDimensionalGate(int effectId, Position targetPos, const std::string& currentTurn, Piece* pieceToMove)
{
    // 1. 基本的なチェック
    if (effectId != 7 || !board->isInsideBoard(targetPos) || !pieceToMove) {
        return false;
    }

    // 2. ワープ元の座標を取得
    Position originalPos = pieceToMove->GetBoardPosition();
    if (originalPos.x == -1) {
        return false;
    }

    // 3. ワープする駒の論理的なChessPiece*を取得
    auto logicPieceToMove = board->getPieceAt(originalPos);
    if (!logicPieceToMove || logicPieceToMove->getColor() != currentTurn) {
        // 自駒でなければ対象外
        return false;
    }

    // =================================================================
    // 4. ターゲットマスに駒がいるかを確認（いる場合はワープ不可）
    // =================================================================
    auto targetLogicPiece = board->getPieceAt(targetPos);

    if (targetLogicPiece) {
        // ターゲットマスに何らかの駒がいる場合、ワープは失敗する
        // デバッグログなど: "次元への扉の効果適用失敗: ワープ先に駒が存在します。"
        return false;
    }

    // 5. 論理的な駒の移動 (Boardオブジェクトを更新)
    // - 元のマスを空にする
    board->setPieceAt(originalPos, nullptr);
    magic->Play({ originalPos.x * 100.0f,10.0f,originalPos.y * 100.0f }, 50);
    // - 移動先のマスに駒を配置し、位置情報も更新
    logicPieceToMove->setPosition(targetPos);
    board->setPieceAt(targetPos, logicPieceToMove);
    magic->Play({ targetPos.x * 100.0f,10.0f,targetPos.y * 100.0f }, 50);
    // =================================================================
    // ★ 6. 【修正】ワープ後の移動不可 (Immobilized) 適用と解除効果の付与
    // =================================================================

    // a. 駒を移動不可状態にする
    // このフラグは、SceneGame::Update の駒選択ロジックで参照されます。
    logicPieceToMove->setImmobilized(true);

    // b. 1ターン後に移動不可を解除するための持続効果を付与
    Position immobilizedTarget = targetPos; // ワープ先の座標をターゲットとする

    // ID 10: 移動不可解除効果 (1ターン持続)
    ActiveEffect unblockEffect(
        10,            // ★新しい効果ID: 10 (移動不可解除)
        1,             // 残りターン数: 1 (次のターン開始時に発動/解除)
        immobilizedTarget, // ターゲット位置
        currentTurn    // 所有者
    );

    // ActiveEffectManagerに登録
    ActiveEffectManager::GetInstance().AddEffect(unblockEffect);

    // 7. 成功
    return true;
}