#pragma once
#include <vector>
#include <string>
#include "ChessPiece.h"

class Knight : public ChessPiece {
public:
    Knight(std::string c, Position p);
    std::string getType() const override;
    std::vector<Position> getLegalMoves(const Board& board, bool isForCheck = false) const override;
    std::shared_ptr<ChessPiece> Knight::clone() const {
        return std::make_shared<Knight>(*this);
    }

};