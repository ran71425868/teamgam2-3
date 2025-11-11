#include "ChessPiece.h"

ChessPiece::ChessPiece(std::string c, Position p, int initialHealth) : color(c), pos(p), health(initialHealth), maxHealth(initialHealth) {}
void ChessPiece::setPosition(Position p) { pos = p; }

// 最大体力を取得するメソッドの実装
int ChessPiece::getMaxHealth() const {
    return maxHealth;
}

// ダメージ処理
void ChessPiece::takeDamage(int damage) {
    health -= damage;
    if (health < 0) {
        health = 0;
    }
}
Position ChessPiece::getPosition() const { return pos; }
std::string ChessPiece::getColor() const { return color; }