#include "Pawn.h"
#include "Board.h"

// Pawn
Pawn::Pawn(std::string c, Position p) : ChessPiece(c, p) {}
std::string Pawn::getType() const { return "Pawn"; }
std::vector<Position> Pawn::getLegalMoves(const Board& board) const {
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

    board.filterPinnedMoves(*this, moves);

    //int direction = (color == "white") ? 1 : -1;
    //moves.push_back({ pos.x, pos.y + direction });
    //// 初期位置なら2マス進める（例：y == 1 for white, y == 6 for black）
    //if ((color == "white" && pos.y == 1) || (color == "black" && pos.y == 6)) {
    //    moves.push_back({ pos.x, pos.y + 2 * direction });
    //}
    //// 斜めの取り
    //moves.push_back({ pos.x + 1, pos.y + direction });
    //moves.push_back({ pos.x - 1, pos.y + direction });
    return moves;
}