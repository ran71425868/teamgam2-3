#include "ChessAI.h"
#include <iostream>
#include "EffectManager.h"

ChessAI::ChessAI()
{
    std::random_device rd;
    gen.seed(rd());

    deleteEffect = new Effect("Data/Effect/Blow11.efk");
}

ChessAI::~ChessAI()
{
    delete deleteEffect;
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
    std::vector<std::pair<Position, Position>> kingKillMoves;

    for (auto& [from, to] : moves) {
        auto attacker = board->getPieceAt(from);
        auto defender = board->getPieceAt(to);

        if (!attacker || !defender) continue;

      
        if (defender->getType() == "King") {

          
            if (attacker->getHealth() > defender->getHealth()) {

                kingKillMoves.push_back({ from, to });
            }
        }
    }

    if (!kingKillMoves.empty()) {
        moves = kingKillMoves;
    }


    bool kingInCheck = board->isKingInCheck("black");

    bool otherPieceHasMoves = false;

    for (int y = 0; y < 8; ++y) {
        if (otherPieceHasMoves) break;
        for (int x = 0; x < 8; ++x) {
            Position from = { x, y };
            auto piece = board->getPieceAt(from);

            if (!piece || piece->getColor() != "black" || piece->getType() == "King") continue;

            auto legalMoves = piece->getLegalMoves(*board);
            for (auto& to : legalMoves) {
                if (to.isValid()) {
                    otherPieceHasMoves = true;
                    break;
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


    std::vector<std::pair<Position, Position>> safeCaptures;

    for (auto& [from, to] : moves) {
        auto attacker = board->getPieceAt(from);
        auto defender = board->getPieceAt(to);

        if (!attacker || !defender)
            continue;

        if (attacker->getColor() == defender->getColor())
            continue;

        if (attacker->getHealth() >= defender->getHealth()) {
            safeCaptures.push_back({ from, to });
        }
    }

    
    if (!safeCaptures.empty()) {
        moves = safeCaptures;
    }
    std::vector<std::pair<Position, Position>> checkMoves;

    for (auto& [from, to] : moves) {
        // à⁄ìÆÇµÇΩÇ∆âºíËÇµÇƒÉ`ÉFÉbÉNÇ≈Ç´ÇÈÇ©ämîF
        if (board->wouldMoveGiveCheck(from, to, "white")) {
            checkMoves.push_back({ from, to });
        }
    }

    // É`ÉFÉbÉNÇ≈Ç´ÇÈéËÇ™Ç†ÇÈÇ»ÇÁÇªÇÍÇç≈óDêÊ
    if (!checkMoves.empty()) {
        moves = checkMoves;
    }


    std::uniform_int_distribution<int> dist(0, (int)moves.size() - 1);
    auto [from, to] = moves[dist(gen)];

    auto attacker = board->getPieceAt(from);
    auto defender = board->getPieceAt(to);

    if (attacker) {
        bool captured = (defender != nullptr);
        bool attackerSurvived = true;

     
        if (defender) {
            int attackerHealth = attacker->getHealth();
            int defenderHealth = defender->getHealth();

            if (attackerHealth < defenderHealth) {
                // çUåÇë§ (AI) ÇÃîsñk: çUåÇë§ÇÕà⁄ìÆÇπÇ∏ÅAè¡ñ≈
                attackerSurvived = false;
                deleteEffect->Play({ from.x * 100.0f,100.0f,from.y * 100.0f }, 30.0f);
                // 1. î’ñ Ç©ÇÁçUåÇë§ÇçÌèú
                board->setPieceAt(from, nullptr);

                // 2. ñhå‰ë§Ç…É_ÉÅÅ[ÉWÇó^Ç¶ÇÈ
                defender->takeDamage(attackerHealth);

                // 3. çUåÇë§ÇÃï`âÊçÌèúÇàÀóä (à⁄ìÆå≥Ç≈ÇÃéÄñSÇé¶Ç∑ÇΩÇﬂÅAfrom->from)
                if (onMoveCallback) onMoveCallback(from, from, false);

                // 4. ñhå‰ë§ÇÃéÄñSÉ`ÉFÉbÉN
                if (defender->getHealth() <= 0) {
                    // ñhå‰ë§Ç‡éÄñSÇµÇΩèÍçáÅAî’ñ Ç©ÇÁçÌèú
                    deleteEffect->Play({ to.x * 100.0f,100.0f,to.y * 100.0f }, 30.0f);
                    board->setPieceAt(to, nullptr);

                    if (onMoveCallback) onMoveCallback(to, to, false);
                }

            }
            else {
                // çUåÇë§ (AI) ÇÃèüóò: ñhå‰ë§Çî’ñ Ç©ÇÁçÌèú
                deleteEffect->Play({to.x * 100.0f,100.0f,to.y * 100.0f }, 30.0f);
                board->setPieceAt(to, nullptr);

            }
        }

        if (attackerSurvived) {

            attacker->takeDamage(1);


            if (attacker->getHealth() <= 0) {
                deleteEffect->Play({ from.x * 100.0f,100.0f,from.y * 100.0f }, 30.0f);
                board->setPieceAt(from, nullptr); // î’ñ Ç©ÇÁçÌèú

                if (onMoveCallback) onMoveCallback(from, from, false);
            }
            else {

                board->movePiece(from, to);
          
                if (onMoveCallback) onMoveCallback(from, to, captured);
            }
        }
    }

    // É^Å[ÉìêÿÇËë÷Ç¶
    board->switchTurn();

    /*bool captured = (board->getPieceAt(to) != nullptr);

    auto movingPiece = board->getPieceAt(from);
    if (movingPiece) {
        movingPiece->takeDamage(1);
    }

    // à⁄ìÆëOÇ…éÄÇÒÇ≈Ç¢ÇΩèÍçá
    if (movingPiece && movingPiece->getHealth() <= 0) {
        board->setPieceAt(from, nullptr);
        if (onMoveCallback) onMoveCallback(from, to, captured);
    }
    else {
        // ê∂Ç´ÇƒÇ¢ÇÍÇŒà⁄ìÆ
        board->movePiece(from, to);
        if (onMoveCallback) onMoveCallback(from, to, captured);
    }

    board->switchTurn();*/
   
}
