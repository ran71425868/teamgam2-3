#include "ChessPiece.h"
#include "Board.h" 

ChessPiece::ChessPiece(std::string c, Position p) : color(c), pos(p) {}
void ChessPiece::setPosition(Position p) { pos = p; }
Position ChessPiece::getPosition() const { return pos; }
std::string ChessPiece::getColor() const { return color; }

//King
King::King(std::string c, Position p) : ChessPiece(c, p) {}
std::string King::getType() const { return "King"; }
std::vector<Position> King::getLegalMoves(const Board& board) const {
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

                    //if (self) {
                    //    self->setPosition(p); // 仮移動時に位置更新
                    //}

                    if (!tempBoard.isKingInCheck(color)) {
                        moves.push_back(p);
                    }

                    tempBoard.setPieceAt(pos, self);
                    tempBoard.setPieceAt(p, nullptr);

                    //if (self) {
                    //    self->setPosition(pos); // 元に戻すときも安全に
                    //}

                }
            }
        }
    }
    /*for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx != 0 || dy != 0) {
                moves.push_back({ pos.x + dx, pos.y + dy });
            }
        }
    }*/
    return moves;
}

//Rook
Rook::Rook(std::string c, Position p) : ChessPiece(c, p) {}
std::string Rook::getType() const { return "Rook"; }
std::vector<Position> Rook::getLegalMoves(const Board& board) const {
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
    board.filterPinnedMoves(*this, moves);
    return moves;

   /* for (int i = 1; i < 8; ++i) {
        moves.push_back({ pos.x + i, pos.y });
        moves.push_back({ pos.x - i, pos.y });
        moves.push_back({ pos.x, pos.y + i });
        moves.push_back({ pos.x, pos.y - i });
    }
    return moves;*/
}

// Bishop
Bishop::Bishop(std::string c, Position p) : ChessPiece(c, p) {}
std::string Bishop::getType() const { return "Bishop"; }
std::vector<Position> Bishop::getLegalMoves(const Board& board) const {
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
    board.filterPinnedMoves(*this, moves);
    return moves;


    /*for (int i = 1; i < 8; ++i) {
        moves.push_back({ pos.x + i, pos.y + i });
        moves.push_back({ pos.x - i, pos.y - i });
        moves.push_back({ pos.x + i, pos.y - i });
        moves.push_back({ pos.x - i, pos.y + i });
    }
    return moves;*/
}

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
        if (!board.getPieceAt(oneStep) && !board.getPieceAt(twoStep))
            moves.push_back(twoStep);
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
