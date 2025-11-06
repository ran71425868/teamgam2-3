#include "Board.h"
#include "Knight.h"

// Knight
Knight::Knight(std::string c, Position p) : ChessPiece(c, p) {}
std::string Knight::getType() const { return "Knight"; }
std::vector<Position> Knight::getLegalMoves(const Board& board) const {
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
    board.filterPinnedMoves(*this, moves);

    /*= {
        {pos.x + 1, pos.y + 2}, {pos.x + 1, pos.y - 2},
        {pos.x - 1, pos.y + 2}, {pos.x - 1, pos.y - 2},
        {pos.x + 2, pos.y + 1}, {pos.x + 2, pos.y - 1},
        {pos.x - 2, pos.y + 1}, {pos.x - 2, pos.y - 1}
    };*/
    return moves;
}
