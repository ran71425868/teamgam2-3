#include "ChessPiece.h"

ChessPiece::ChessPiece(std::string c, Position p, int initialHealth) : color(c), pos(p), health(initialHealth), maxHealth(initialHealth) {}
void ChessPiece::setPosition(Position p) { pos = p; }

// 最大体力を取得するメソッドの実装
int ChessPiece::getMaxHealth() const {
    return maxHealth;
}

// 体力を回復させるメソッドの実装
void ChessPiece::heal(int amount) {
    if (amount <= 0) return; // 負の値やゼロでは回復しない

    // 現在の体力に回復量を加える
    int tentativeHealth = health + amount;

    // 最大体力(maxHealth)を超えないように制限
    if (tentativeHealth > maxHealth) {
        health = maxHealth;
    }
    else {
        health = tentativeHealth;
    }
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