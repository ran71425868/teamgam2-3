#include "Board.h"
#include "Bishop.h"

// Bishop
Bishop::Bishop(std::string c, Position p) : ChessPiece(c, p, 5) {}
std::string Bishop::getType() const { return "Bishop"; }
std::vector<Position> Bishop::getLegalMoves(const Board& board, bool isForCheck) const {
    // 移動制限が適用されているかチェック
    if (IsImmobilized) {
        return {}; // 空のリストを返す (移動不可)
    }
    std::vector<Position> moves;
    const int directions[4][2] = { {1,1},{-1,-1},{1,-1},{-1,1} };

    for (auto& dir : directions) {
        int x = pos.x + dir[0];
        int y = pos.y + dir[1];
        while (board.isInsideBoard({ x, y })) {
            auto target = board.getPieceAt({ x, y });
            if (target) {
                if (target->getColor() != color)
                    moves.push_back({ x, y }); // 敵なら取れる
                break; // 味方でも敵でも止まる
            }
            moves.push_back({ x, y });
            x += dir[0];
            y += dir[1];
        }
    }

    // ピン判定でフィルタリング
    if (!isForCheck) {
        board.filterPinnedMoves(*this, moves);
    }
    return moves;
}

std::shared_ptr<ChessPiece> Bishop::clone() const {
    return std::make_shared<Bishop>(*this);
}
