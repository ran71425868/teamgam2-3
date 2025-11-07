#include "Board.h"
#include "Bishop.h"

// Bishop
Bishop::Bishop(std::string c, Position p) : ChessPiece(c, p) {}
std::string Bishop::getType() const { return "Bishop"; }
std::vector<Position> Bishop::getLegalMoves(const Board& board, bool isForCheck) const {
    std::vector<Position> moves;
    const int directions[4][2] = { {1,1},{-1,-1},{1,-1},{-1,1} };

    for (auto& dir : directions) {
        int x = pos.x + dir[0];
        int y = pos.y + dir[1];
        while (board.isInsideBoard({ x, y })) {
            auto target = board.getPieceAt({ x, y });
            if (target) {
                if (target->getColor() != color)
                    moves.push_back({ x, y }); // “G‚È‚çŽæ‚ê‚é
                break; // –¡•û‚Å‚à“G‚Å‚àŽ~‚Ü‚é
            }
            moves.push_back({ x, y });
            x += dir[0];
            y += dir[1];
        }
    }

    // ƒsƒ“”»’è‚ÅƒtƒBƒ‹ƒ^ƒŠƒ“ƒO
    if (!isForCheck) {
        board.filterPinnedMoves(*this, moves);
    }
    return moves;
}