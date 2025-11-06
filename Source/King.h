#pragma once
#include <vector>
#include <string>
#include "ChessPiece.h"


class King : public ChessPiece {
public:
    King(std::string c, Position p);
    std::string getType() const override;
    std::vector<Position> getLegalMoves(const Board& board, bool isForCheck = false) const override;
};