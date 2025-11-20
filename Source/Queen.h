#pragma once
#include "ChessPiece.h"

class Queen : public ChessPiece {
public:
    Queen(std::string c, Position p);
    std::string getType() const override;
    std::vector<Position> getLegalMoves(const Board& board, bool isForCheck = false) const override;
    std::shared_ptr<ChessPiece> Queen::clone() const {
        return std::make_shared<Queen>(*this);
    }

};