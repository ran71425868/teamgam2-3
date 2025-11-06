#pragma once
#include "ChessPiece.h"

class Queen : public ChessPiece {
public:
    Queen(std::string c, Position p);
    std::string getType() const override;
    std::vector<Position> getLegalMoves(const Board& board, bool isForCheck = false) const override;
};