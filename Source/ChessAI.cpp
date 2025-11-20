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
found:;



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

    std::uniform_int_distribution<int> dist(0, (int)moves.size() - 1);
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
    }

    board->switchTurn();
}
