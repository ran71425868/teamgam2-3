#pragma once
#include <vector>
#include <string>
#include "ChessPiece.h"

class Knight : public ChessPiece {
public:
    Knight(std::string c, Position p);
    std::string getType() const override;
    std::vector<Position> getLegalMoves(const Board& board) const override;
};