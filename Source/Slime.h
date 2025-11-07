#pragma once
#include "System/ModelRenderer.h"
#include "Board.h"

class Slime {
public:
    Slime(std::string color, Position boardPos,std::string pieceType);
    ~Slime();

    void Update(float elapsedTime);
    void Render(const RenderContext& rc, ModelRenderer* renderer);

    void SetBoardPosition(Position pos);
    Position GetBoardPosition() const;

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

    DirectX::XMFLOAT3  scale = { 1,1,1 };
    std::string color;
    Position boardPosition; // 盤面上の座標（0〜7）
    std::string pieceType;

};
