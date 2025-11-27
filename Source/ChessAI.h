#pragma once
#include "Board.h"
#include <random>
#include <vector>
#include <functional>

class ChessAI
{
public:
    ChessAI();
    void Update(Board* board);  // 黒のターンで自動行動

    // SceneGame 側がコールバックを登録できる (from, to)
    std::function<void(Position from, Position to, bool wasCapture)> onMoveCallback;

private:
    bool hasMoved = false; // このターンですでに行動したか
    std::mt19937 gen;

    void MakeRandomMove(Board* board);

    bool ChessAI::isMoveSafe(Board* board, Position from, Position to) {
        auto captured = board->getPieceAt(to);

     
        board->movePiece(from, to);

        bool safe = !board->isKingInCheck("black");

      
        board->movePiece(to, from);
        board->setPieceAt(to, captured);

        return safe;
    }

};
