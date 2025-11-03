#include "Slime.h"

Slime::Slime(std::string c, Position boardPos) : color(c), boardPosition(boardPos) {
    model = new Model("Data/Model/Slime/Slime.mdl");
    black_bord = new Model("Data/Model/Stage/black_bord.mdl");
    white_bord = new Model("Data/Model/Stage/white_bord.mdl");
    //モデルが大きいのでスケーリング
    scale.x = scale.y = scale.z = 1.0f;
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

    for (int i = 0; i <= 7; i++)
    {
        for (int j = 0; j <= 7; j++)
        {
            DirectX::XMFLOAT4X4 bord_transform;
            DirectX::XMStoreFloat4x4(&bord_transform,
                DirectX::XMMatrixTranslation(
                    i * 100.0f, // 盤面1マス = 100ユニット
                    0.0f,
                    j * 100.0f
                )
            );

            if ((static_cast<int>(i) + static_cast<int>(j)) % 2 == 1)
                renderer->Render(rc, bord_transform, white_bord, ShaderId::Lambert);
            else
                renderer->Render(rc, bord_transform, black_bord, ShaderId::Lambert);
        }
    }
}

void Slime::SetBoardPosition(Position pos) {
    boardPosition = pos;
}

Position Slime::GetBoardPosition() const {
    return boardPosition;
}