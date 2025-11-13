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

    for (int y = 0; y < 8; ++y)
    {
        for (int x = 0; x < 8; ++x)
        {
            Position from = { x, y };
            auto piece = board->getPieceAt(from);
            if (piece && piece->getColor() == "black")
            {
                auto legalMoves = piece->getLegalMoves(*board);
                for (auto& to : legalMoves)
                {
                    if (to.isValid())
                        moves.push_back({ from, to });
                }
            }
        }
    }

    if (moves.empty()) return;

    std::uniform_int_distribution<int> dist(0, (int)moves.size() - 1);
    auto [from, to] = moves[dist(gen)];

    // キャプチャ対象を事前に取得（視覚オブジェクトの削除に使うため）
    auto capturedBefore = board->getPieceAt(to) != nullptr;

    // 盤面を動かす
    board->movePiece(from, to);

    auto movedPiece = board->getPieceAt(to);
    if (movedPiece) {
        movedPiece->takeDamage(1);

        if (movedPiece->getHealth() <= 0) {
            // 死亡：盤面から削除
            board->setPieceAt(to, nullptr);

            // コールバックが設定されていれば、視覚的な削除通知を送る
            if (onMoveCallback) {
                onMoveCallback(from, to, true); // "true" は削除通知として利用可能
            }
        }
    }
  
    if (onMoveCallback) {
        onMoveCallback(from, to, capturedBefore);
    }

    board->switchTurn();
}
