#pragma once
#include "System/ModelRenderer.h"
#include "Board.h"

class Piece {
public:
    Piece(std::string color, Position boardPos, std::string pieceType);
    ~Piece();

    void Update(float elapsedTime);
    void Render(const RenderContext& rc, ModelRenderer* renderer);

    void SetBoardPosition(Position pos);
    Position GetBoardPosition() const;

    void setDisplayInfo(bool isSelected, int currentHealth, int maxHealth);
private:
    //白駒
    Model* white_bishop = nullptr;
    Model* white_pawn = nullptr;
    Model* white_king = nullptr;
    Model* white_knight = nullptr;
    Model* white_queen = nullptr;
    Model* white_rook = nullptr;

    //黒駒
    Model* black_bishop = nullptr;
    Model* black_pawn = nullptr;
    Model* black_king = nullptr;
    Model* black_knight = nullptr;
    Model* black_queen = nullptr;
    Model* black_rook = nullptr;

    //ボード
    Model* black_bord = nullptr;
    Model* white_bord = nullptr;

    std::string color;
    Position boardPosition; // 盤面上の座標（0〜7）
    std::string pieceType;

    // 体力バー表示用の情報
    bool isSelected = false;
    int currentHealth = 0;
    int maxHealth = 1;
};
