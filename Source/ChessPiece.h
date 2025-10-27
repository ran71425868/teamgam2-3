#pragma once

#include <vector>
#include <string>
#include <memory>

struct Position {
    int x, y;
};

class Board; // ëOï˚êÈåæ

class ChessPiece {
protected:
    std::string color;
    Position pos;

public:
    ChessPiece(std::string c, Position p);
    virtual ~ChessPiece() {}

    virtual std::string getType() const = 0;
    virtual std::vector<Position> getLegalMoves(const Board& board) const = 0;

    void setPosition(Position p);
    Position getPosition() const;
    std::string getColor() const;
};

class King : public ChessPiece {
public:
    King(std::string c, Position p);

    std::string getType() const override;

    std::vector<Position> getLegalMoves(const Board& board) const override;
};

class Rook : public ChessPiece {
public:
    Rook(std::string c, Position p);

    std::string getType() const override;

    std::vector<Position> getLegalMoves(const Board& board) const override;
};

class Bishop : public ChessPiece {
public:
    Bishop(std::string c, Position p);
    std::string getType() const override;
    std::vector<Position> getLegalMoves(const Board& board) const override;
};

class Knight : public ChessPiece {
public:
    Knight(std::string c, Position p);
    std::string getType() const override;
    std::vector<Position> getLegalMoves(const Board& board) const override;
};

class Queen : public ChessPiece {
public:
    Queen(std::string c, Position p);
    std::string getType() const override;
    std::vector<Position> getLegalMoves(const Board& board) const override;
};

class Pawn : public ChessPiece {
public:
    Pawn(std::string c, Position p);
    std::string getType() const override;
    std::vector<Position> getLegalMoves(const Board& board) const override;
};
