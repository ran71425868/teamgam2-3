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
    Board::Board(const Board& other) {
        // grid の浅いコピー（shared_ptr をそのままコピー）
        grid = other.grid;
        // 他のメンバーも必要に応じてコピー
    }
    void initialize(); // 初期配置
    void printBoard() const;
    std::shared_ptr<ChessPiece> getPieceAt(Position pos) const;
    void setPieceAt(Position pos, std::shared_ptr<ChessPiece> piece);
    void movePiece(Position from, Position to);
    //bool isInsideBoard(Position pos) const;
    bool  isInsideBoard(const Position& pos) const;
    // ターン管理
    std::string getCurrentTurn() const;
    void switchTurn();

    // チェック・チェックメイト判定
    bool isKingInCheck(std::string color) const;
    bool isCheckmate(std::string color) const;

    // 指定された色のキングがボード上に存在するかをチェックする
    bool isKingPresent(std::string color) const;

    // ピン判定用
    void filterPinnedMoves(const ChessPiece& piece, std::vector<Position>& moves) const;

};
