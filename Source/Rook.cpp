#include "ChessPiece.h"
#include "Board.h" 
#include "Rook.h"


//Rook
Rook::Rook(std::string c, Position p) : ChessPiece(c, p, 5) {}
std::string Rook::getType() const { return "Rook"; }
std::vector<Position> Rook::getLegalMoves(const Board& board, bool isForCheck) const {
    std::vector<Position> moves;
    const int dirs[4][2] = { {1,0},{-1,0},{0,1},{0,-1} };
    for (auto& d : dirs) {
        int x = pos.x + d[0], y = pos.y + d[1];
        while (board.isInsideBoard({ x, y })) {
            auto target = board.getPieceAt({ x, y });
            if (target) {
                if (target->getColor() != color) moves.push_back({ x, y });
                break;
            }
            moves.push_back({ x, y });
            x += d[0]; y += d[1];
        }
    }
    // チェック判定のための呼び出し（再帰）でなければ、ピンされた手をフィルタリングする
    if (!isForCheck) {
        board.filterPinnedMoves(*this, moves);
    }
    return moves;
}