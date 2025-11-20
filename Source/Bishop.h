#pragma once
#include <vector>
#include <string>
#include "ChessPiece.h"

class Bishop : public ChessPiece {
public:
    Bishop(std::string c, Position p);
    std::string getType() const override;
    std::vector<Position> getLegalMoves(const Board& board, bool isForCheck = false) const override;
    std::shared_ptr<ChessPiece> Bishop::clone() const {
        return std::make_shared<Bishop>(*this);
    }

};