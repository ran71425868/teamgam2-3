#include "Pawn.h"
#include "Board.h"

// Pawn
// 例: Pawnの初期体力を3に設定
Pawn::Pawn(std::string c, Position p) : ChessPiece(c, p, 5) {}
std::string Pawn::getType() const { return "Pawn"; }
std::vector<Position> Pawn::getLegalMoves(const Board& board, bool isForCheck) const {
    std::vector<Position> moves;

    int dir = (color == "white") ? 1 : -1;
    Position oneStep = { pos.x, pos.y + dir };
    if (board.isInsideBoard(oneStep) && !board.getPieceAt(oneStep))
        moves.push_back(oneStep);

    // 初期位置なら2マス進める
    Position twoStep = { pos.x, pos.y + 2 * dir };
    if ((color == "white" && pos.y == 1) || (color == "black" && pos.y == 6)) {
        if (board.isInsideBoard(twoStep) && // ★ボード内チェックを追加
            !board.getPieceAt(oneStep) && !board.getPieceAt(twoStep))
        {
            moves.push_back(twoStep);
        }
    }

    // 斜めの取り
    for (int dx : {-1, 1}) {
        Position diag = { pos.x + dx, pos.y + dir };
        if (board.isInsideBoard(diag)) {
            auto target = board.getPieceAt(diag);
            if (target && target->getColor() != color)
                moves.push_back(diag);
        }
    }

    if (!isForCheck) {
        board.filterPinnedMoves(*this, moves);
    }
    return moves;
}