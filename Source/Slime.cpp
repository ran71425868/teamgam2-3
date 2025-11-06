#include "Slime.h"
#include <Pawn.h>

Slime::Slime(std::string c, Position boardPos) : color(c), boardPosition(boardPos) {
    //白駒
    white_bishop = new Model("Data/Model/Slime/white_bishop.mdl");
    white_pawn = new Model("Data/Model/Slime/white_pawn.mdl");
    white_king = new Model("Data/Model/Slime/white_king.mdl");
    white_knight = new Model("Data/Model/Slime/white_knight.mdl");
    white_queen = new Model("Data/Model/Slime/white_queen.mdl");
    white_rook = new Model("Data/Model/Slime/white_rook.mdl");

    //黒駒
    black_bishop = new Model("Data/Model/Slime/black_bishop.mdl");
    black_pawn = new Model("Data/Model/Slime/black_pawn.mdl");
    black_king = new Model("Data/Model/Slime/black_king.mdl");
    black_knight = new Model("Data/Model/Slime/black_knight.mdl");
    black_queen = new Model("Data/Model/Slime/black_queen.mdl");
    black_rook = new Model("Data/Model/Slime/black_rook.mdl");

    //ボード
    black_bord = new Model("Data/Model/Stage/black_bord.mdl");
    white_bord = new Model("Data/Model/Stage/white_bord.mdl");

    //モデルが大きいのでスケーリング
    scale.x = scale.y = scale.z = 1.0f;
}

Slime::~Slime() {
    //白駒
    delete white_bishop;
    delete white_pawn;
    delete white_king;
    delete white_knight;
    delete white_queen;
    delete white_rook;

    //黒駒
    delete black_bishop;
    delete black_pawn;
    delete black_king;
    delete black_knight;
    delete black_queen;
    delete black_rook;
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

    if (color == "white")
    {
        //renderer->Render(rc, transform, white_bishop, ShaderId::Lambert);
        renderer->Render(rc, transform, white_pawn, ShaderId::Lambert);
        //renderer->Render(rc, transform, white_king, ShaderId::Lambert);
        //renderer->Render(rc, transform, white_knight, ShaderId::Lambert);
        //renderer->Render(rc, transform, white_queen, ShaderId::Lambert);
       // renderer->Render(rc, transform, white_rook, ShaderId::Lambert);

    }
    else
    {
        //renderer->Render(rc, transform, black_bishop, ShaderId::Lambert);
        renderer->Render(rc, transform, black_pawn, ShaderId::Lambert);
        //renderer->Render(rc, transform, black_king, ShaderId::Lambert);
        //renderer->Render(rc, transform, black_knight, ShaderId::Lambert);
        //renderer->Render(rc, transform, black_queen, ShaderId::Lambert);
        //renderer->Render(rc, transform, black_rook, ShaderId::Lambert);

    }

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