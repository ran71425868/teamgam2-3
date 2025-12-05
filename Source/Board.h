#pragma once
#include <memory>
#include <vector>
#include "ChessPiece.h"
#include "System/Model.h"
#include "System/ModelRenderer.h"

enum class HealType {
    NONE,       // 回復マスではない
    WHITE_ONLY, // 白駒専用（黒陣地側に生成）
    BLACK_ONLY, // 黒駒専用（白陣地側に生成）
    COMMON      // 共通マス（両方回復可能）
};

// 回復マスの情報を保持する構造体
struct HealSpot {
    HealType type = HealType::NONE;
    bool isGenerated = false; // マスが生成済みかどうか
};

class Board {
private:
    std::vector<std::vector<std::shared_ptr<ChessPiece>>> grid;
    std::string currentTurn; // "white" or "black"

    Model* black_bord = nullptr;
    Model* white_bord = nullptr;
public:
    Board();
    Board::Board(const Board& other) {
        currentTurn = other.currentTurn;

        grid.resize(8, std::vector<std::shared_ptr<ChessPiece>>(8));

        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                if (other.grid[y][x]) {
                    grid[y][x] = other.grid[y][x]->clone();
                }
                else {
                    grid[y][x] = nullptr;
                }
            }
        }

        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                if (grid[y][x]) {
                    grid[y][x]->setPosition({ x, y });
                }
            }
        }
    }
    void initialize(); // 初期配置
    void tutorial();
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


    void Render(const RenderContext& rc, ModelRenderer* renderer);

    bool wouldMoveGiveCheck(Position from, Position to, const std::string& enemyColor);

};
