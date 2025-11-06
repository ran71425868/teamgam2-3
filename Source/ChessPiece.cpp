#include "ChessPiece.h"

ChessPiece::ChessPiece(std::string c, Position p) : color(c), pos(p) {}
void ChessPiece::setPosition(Position p) { pos = p; }
Position ChessPiece::getPosition() const { return pos; }
std::string ChessPiece::getColor() const { return color; }