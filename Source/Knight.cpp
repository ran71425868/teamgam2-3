#include "Board.h"
#include "Knight.h"

// Knight
Knight::Knight(std::string c, Position p) : ChessPiece(c, p, 5) {}
std::string Knight::getType() const { return "Knight"; }
std::vector<Position> Knight::getLegalMoves(const Board& board, bool isForCheck) const {
    std::vector<Position> moves;
    const int jumps[8][2] = {
        {1,2},{2,1},{-1,2},{-2,1},
        {1,-2},{2,-1},{-1,-2},{-2,-1}
    };
    for (auto& j : jumps) {
        Position p = { pos.x + j[0], pos.y + j[1] };
        if (board.isInsideBoard(p)) {
            auto target = board.getPieceAt(p);
            if (!target || target->getColor() != color)
                moves.push_back(p);
        }
    }
    if (!isForCheck) {
        board.filterPinnedMoves(*this, moves);
    }
    
    return moves;
}
