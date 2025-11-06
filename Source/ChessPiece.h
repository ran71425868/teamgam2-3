#pragma once

#include <vector>
#include <string>
#include <memory>

struct Position {
    int x, y;

    bool isValid() const {
        return x >= 0 && x < 8 && y >= 0 && y < 8;
    }

};

class Board; // ‘O•ûéŒ¾

class ChessPiece {
protected:
    std::string color;
    Position pos;

public:
    ChessPiece(std::string c, Position p);
    virtual ~ChessPiece() {}

    virtual std::string getType() const = 0;
    virtual std::vector<Position> getLegalMoves(const Board& board, bool isForCheck = false) const=0;

    void setPosition(Position p);
    Position getPosition() const;
    std::string getColor() const;
};