#pragma once
#include "ChessPiece.h"

class Rook : public ChessPiece {
public:
    Rook(std::string c, Position p);
    std::string getType() const override;
    std::vector<Position> getLegalMoves(const Board& board, bool isForCheck = false) const override;
    std::shared_ptr<ChessPiece> Rook::clone() const {
        return std::make_shared<Rook>(*this);
    }

};