#include "ChessAI.h"
#include <iostream>

ChessAI::ChessAI()
{
    std::random_device rd;
    gen.seed(rd());
}

void ChessAI::Update(Board* board)
{
    if (board->getCurrentTurn() != "black")
    {
        hasMoved = false;
        return;
    }

    if (!hasMoved)
    {
        MakeRandomMove(board);
        hasMoved = true;
    }
}

void ChessAI::MakeRandomMove(Board* board)
{
    std::vector<std::pair<Position, Position>> moves;

    bool kingInCheck = board->isKingInCheck("black");

    bool otherPieceHasMoves = false;

    for (int y = 0; y < 8; ++y) {
        if (otherPieceHasMoves) break; // 外側ループを脱出
        for (int x = 0; x < 8; ++x) {
            Position from = { x, y };
            auto piece = board->getPieceAt(from);

            if (!piece || piece->getColor() != "black" || piece->getType() == "King") continue;

            auto legalMoves = piece->getLegalMoves(*board);
            for (auto& to : legalMoves) {
                if (to.isValid()) {
                    otherPieceHasMoves = true;
                    break; // 内側ループを脱出
                }
            }
        }
    }

    /* for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            Position from = { x, y };
            auto piece = board->getPieceAt(from);

            if (!piece) continue;
            if (piece->getColor() != "black") continue;
            if (piece->getType() == "King") continue;

            auto legalMoves = piece->getLegalMoves(*board);
            for (auto& to : legalMoves) {
                if (to.isValid()) {
                    otherPieceHasMoves = true;
                    goto found;
                }
            }
        }
    }
found:;*/

    std::vector<std::pair<Position, Position>> allMoves;

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            Position from = { x, y };
            auto piece = board->getPieceAt(from);

            if (!piece) continue;
            if (piece->getColor() != "black") continue;

            if (!kingInCheck && otherPieceHasMoves && piece->getType() == "King")
                continue;

            auto legalMoves = piece->getLegalMoves(*board);
            for (auto& to : legalMoves) {
                if (to.isValid()) {
                    allMoves.push_back({ from, to });
                }
            }
        }
    }


    if (kingInCheck) {
        for (auto& [from, to] : allMoves) {
            if (isMoveSafe(board, from, to)) {
                moves.push_back({ from, to });
            }
        }
    }
    else {
        moves = allMoves;
    }


    if (moves.empty()) return;

   /* std::uniform_int_distribution<int> dist(0, (int)moves.size() - 1);
    auto [from, to] = moves[dist(gen)];

    bool capturedBefore = (board->getPieceAt(to) != nullptr);

    board->movePiece(from, to);

    auto movedPiece = board->getPieceAt(to);
    if (movedPiece) {
        movedPiece->takeDamage(1);
        if (movedPiece->getHealth() <= 0) {
            board->setPieceAt(to, nullptr);

            if (onMoveCallback) {
                onMoveCallback(from, to, true);
            }
        }
    }

    if (onMoveCallback) {
        onMoveCallback(from, to, capturedBefore);
    }*/

    std::uniform_int_distribution<int> dist(0, (int)moves.size() - 1);
    auto [from, to] = moves[dist(gen)];

    auto attacker = board->getPieceAt(from); // 動かす駒 (AI)
    auto defender = board->getPieceAt(to);   // 取られる駒 (Player)

    if (attacker) {
        bool captured = (defender != nullptr);
        bool attackerSurvived = true;

        // --- A. 戦闘判定 ---
        if (defender) {
            int attackerHealth = attacker->getHealth();
            int defenderHealth = defender->getHealth();

            if (attackerHealth < defenderHealth) {
                // 攻撃側 (AI) の敗北: 攻撃側は移動せず、消滅
                attackerSurvived = false;

                // 1. 盤面から攻撃側を削除
                board->setPieceAt(from, nullptr);

                // 2. 防御側にダメージを与える
                defender->takeDamage(attackerHealth);

                // 3. 攻撃側の描画削除を依頼 (移動元での死亡を示すため、from->from)
                if (onMoveCallback) onMoveCallback(from, from, false);

                // 4. 防御側の死亡チェック
                if (defender->getHealth() <= 0) {
                    // 防御側も死亡した場合、盤面から削除
                    board->setPieceAt(to, nullptr);
                    // SceneGameに防御側の描画削除を依頼 (toの位置の削除を依頼)
                    // ★ captured=true 相当の処理が必要だが、ここでは from->to, true で代用
                    // ただし、attackerは移動していないので、toの削除だけを依頼する専用コールバックがないと難しい。
                    // 暫定的に onMoveCallback(to, to, false) で to の位置の描画削除を依頼する
                    if (onMoveCallback) onMoveCallback(to, to, false);
                }

            }
            else {
                // 攻撃側 (AI) の勝利: 防御側を盤面から削除
                board->setPieceAt(to, nullptr);
                // SceneGameに防御側の描画を削除させる (captured = true, onMoveCallback(from, to, true) で処理される)

            }
        }

        // --- B. 移動と自傷ダメージ ---
        if (attackerSurvived) {

            // 1. 移動する前に自傷ダメージ
            attacker->takeDamage(1);

            // 2. 自傷ダメージで死亡した場合
            if (attacker->getHealth() <= 0) {
                board->setPieceAt(from, nullptr); // 盤面から削除
                // 描画の削除は、移動が起こらないため from の位置の削除をコールバックで依頼
                if (onMoveCallback) onMoveCallback(from, from, false); // from->from (死亡)
            }
            else {
                // 3. 生きていれば移動を実行
                board->movePiece(from, to);
                // SceneGameに移動を通知 (描画オブジェクトの位置更新のため)
                if (onMoveCallback) onMoveCallback(from, to, captured);
            }
        }
    }

    // ターン切り替え
    board->switchTurn();

    /*bool captured = (board->getPieceAt(to) != nullptr);

    auto movingPiece = board->getPieceAt(from);
    if (movingPiece) {
        movingPiece->takeDamage(1);
    }

    // 移動前に死んでいた場合
    if (movingPiece && movingPiece->getHealth() <= 0) {
        board->setPieceAt(from, nullptr);
        if (onMoveCallback) onMoveCallback(from, to, captured);
    }
    else {
        // 生きていれば移動
        board->movePiece(from, to);
        if (onMoveCallback) onMoveCallback(from, to, captured);
    }

    board->switchTurn();*/
   
}
