#pragma once
#include <vector>
#include <string>
#include "ChessPiece.h"

class Pawn : public ChessPiece {
public:
    Pawn(std::string c, Position p);
    std::string getType() const override;
    std::vector<Position> getLegalMoves(const Board& board, bool isForCheck = false) const override;
    std::shared_ptr<ChessPiece> clone() const override;

};