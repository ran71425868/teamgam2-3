#include "Queen.h"
#include "Board.h"

// Queen
Queen::Queen(std::string c, Position p) : ChessPiece(c, p) {}
std::string Queen::getType() const { return "Queen"; }
std::vector<Position> Queen::getLegalMoves(const Board& board) const {
    std::vector<Position> moves;
    const int dirs[8][2] = { {1,0},{-1,0},{0,1},{0,-1},{1,1},{-1,-1},{1,-1},{-1,1} };
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
    board.filterPinnedMoves(*this, moves);
    /*for (int i = 1; i < 8; ++i) {
        moves.push_back({ pos.x + i, pos.y });
        moves.push_back({ pos.x - i, pos.y });
        moves.push_back({ pos.x, pos.y + i });
        moves.push_back({ pos.x, pos.y - i });
        moves.push_back({ pos.x + i, pos.y + i });
        moves.push_back({ pos.x - i, pos.y - i });
        moves.push_back({ pos.x + i, pos.y - i });
        moves.push_back({ pos.x - i, pos.y + i });
    }*/
    return moves;
}