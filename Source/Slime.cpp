#include "Slime.h"

Slime::Slime(std::string c, Position boardPos) : color(c), boardPosition(boardPos) {
    model = new Model("Data/Model/Slime/Slime.mdl");
    //モデルが大きいのでスケーリング
    scale.x = scale.y = scale.z = 0.01f;
}

Slime::~Slime() {
    delete model;
}

void Slime::Update(float elapsedTime) {
    // アニメーションや移動補間があればここに
}

void Slime::Render(const RenderContext& rc, ModelRenderer* renderer) {
    DirectX::XMFLOAT4X4 transform;
    DirectX::XMStoreFloat4x4(&transform,
        DirectX::XMMatrixTranslation(
            boardPosition.x * 100.0f, // 盤面1マス = 100ユニット
            0.0f,
            boardPosition.y * 100.0f
        )
    );

    renderer->Render(rc, transform, model, ShaderId::Lambert);
}

void Slime::SetBoardPosition(Position pos) {
    boardPosition = pos;
}

Position Slime::GetBoardPosition() const {
    return boardPosition;
}