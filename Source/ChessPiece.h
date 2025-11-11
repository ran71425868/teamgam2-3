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

class Board; // 前方宣言

class ChessPiece {
protected:
    std::string color;
    Position pos;

    int health;
    int maxHealth;

public:
    ChessPiece(std::string c, Position p, int initialHealth);
    virtual ~ChessPiece() {}

    virtual std::string getType() const = 0;
    virtual std::vector<Position> getLegalMoves(const Board& board, bool isForCheck = false) const=0;

    void setPosition(Position p);

    // ★新規追加：最大体力を取得するメソッド
    int getMaxHealth() const;

    int getHealth() const { return health; }
    void takeDamage(int damage);

    // 最大体力を設定するメソッド（各駒のコンストラクタで呼び出す）
    //void setMaxHealth(int maxH) { maxHealth = maxH; }

    Position getPosition() const;
    std::string getColor() const;
};