#include "Board.h" 
#include "King.h"

//King
King::King(std::string c, Position p) : ChessPiece(c, p) {}
std::string King::getType() const { return "King"; }
std::vector<Position> King::getLegalMoves(const Board& board, bool isForCheck) const {
    std::vector<Position> moves;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;
            Position p = { pos.x + dx, pos.y + dy };
            if (board.isInsideBoard(p)) {
                auto target = board.getPieceAt(p);
                if (!target || target->getColor() != color) {
                    // 仮に動かしてチェックされるか確認
                    // Board を一時コピーして仮移動
                    Board tempBoard = board;
                    auto self = tempBoard.getPieceAt(pos);
                    tempBoard.setPieceAt(p, self);
                    tempBoard.setPieceAt(pos, nullptr);

                    /*if (!tempBoard.isKingInCheck(color)) {
                    }*/
                        moves.push_back(p);

                }
            }
        }
    }
    return moves;
}