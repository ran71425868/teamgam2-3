#pragma once
#include "System/ModelRenderer.h"
#include "Board.h"

class Slime {
public:
    Slime(std::string color, Position boardPos);
    ~Slime();

    void Update(float elapsedTime);
    void Render(const RenderContext& rc, ModelRenderer* renderer);

    void SetBoardPosition(Position pos);
    Position GetBoardPosition() const;

private:
    Model* model = nullptr;
    Model* black_bord = nullptr;
    Model* white_bord = nullptr;
    DirectX::XMFLOAT3  scale = { 1,1,1 };
    std::string color;
    Position boardPosition; // 盤面上の座標（0〜7）
};
