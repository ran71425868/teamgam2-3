#pragma once
#include <memory>
#include <vector>
#include "ChessPiece.h"

class Board {
private:
    std::vector<std::vector<std::shared_ptr<ChessPiece>>> grid;
    std::string currentTurn; // "white" or "black"


public:
    Board();

    void initialize(); // 初期配置
    void printBoard() const;
    std::shared_ptr<ChessPiece> getPieceAt(Position pos) const;
    void setPieceAt(Position pos, std::shared_ptr<ChessPiece> piece);
    void movePiece(Position from, Position to);
    bool isInsideBoard(Position pos) const;

    // ターン管理
    std::string getCurrentTurn() const;
    void switchTurn();

    // チェック・チェックメイト判定
    bool isKingInCheck(std::string color) const;
    bool isCheckmate(std::string color);

    // ピン判定用
    void filterPinnedMoves(const ChessPiece& piece, std::vector<Position>& moves) const;


};
