#include "Piece.h"

Piece::Piece(std::string c, Position boardPos, std::string pType) : color(c), boardPosition(boardPos), pieceType(pType) {
    //白駒
    white_bishop = new Model("Data/Model/Chesspiece/bishop_white.mdl");
    white_pawn = new Model("Data/Model/Chesspiece/pown_white.mdl");
    white_king = new Model("Data/Model/Chesspiece/king_white.mdl");
    white_knight = new Model("Data/Model/Chesspiece/knight_white.mdl");
    white_queen = new Model("Data/Model/Chesspiece/queen_white.mdl");
    white_rook = new Model("Data/Model/Chesspiece/rook_white.mdl");

    //黒駒
    black_bishop = new Model("Data/Model/Chesspiece/bishop_black.mdl");
    black_pawn = new Model("Data/Model/Chesspiece/pown_black.mdl");
    black_king = new Model("Data/Model/Chesspiece/king_black.mdl");
    black_knight = new Model("Data/Model/Chesspiece/knight_black.mdl");
    black_queen = new Model("Data/Model/Chesspiece/queen_black.mdl");
    black_rook = new Model("Data/Model/Chesspiece/rook_black.mdl");
    //ボード
    black_bord = new Model("Data/Model/Stage/black_bord.mdl");
    white_bord = new Model("Data/Model/Stage/white_bord.mdl");

    worldPos = BoardToWorld(boardPos);

    moveEffect = new Effect("Data/Effect/move.efk");

}

Piece::~Piece() {
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

    delete moveEffect;
}

void Piece::Update(float elapsedTime) {
    // アニメーションや移動補間があればここに

    if (isMoving) {
        animTime += elapsedTime;

        float t = animTime / animDuration;
        if (t >= 1.0f) {
            t = 1.0f;
            isMoving = false;
        }

        worldPos.x = startPos.x * (1 - t) + targetPos.x * t;
        worldPos.y = startPos.y * (1 - t) + targetPos.y * t;
        worldPos.z = startPos.z * (1 - t) + targetPos.z * t;
    }
}

void Piece::Render(const RenderContext& rc, ModelRenderer* renderer) {
    DirectX::XMFLOAT4X4 transform;

    DirectX::XMStoreFloat4x4(&transform,
        DirectX::XMMatrixTranslation(
            worldPos.x * 100.0f, // 盤面1マス = 100ユニット
            0.0f,
            worldPos.z * 100.0f
        )
    );

    if (color == "white")
    {
        if (pieceType == "bishop")
            renderer->Render(rc, transform, white_bishop, ShaderId::Lambert);
        if (pieceType == "pawn")
            renderer->Render(rc, transform, white_pawn, ShaderId::Lambert);
        if (pieceType == "king")
            renderer->Render(rc, transform, white_king, ShaderId::Lambert);
        if (pieceType == "knight")
            renderer->Render(rc, transform, white_knight, ShaderId::Lambert);
        if (pieceType == "queen")
            renderer->Render(rc, transform, white_queen, ShaderId::Lambert);
        if (pieceType == "rook")
            renderer->Render(rc, transform, white_rook, ShaderId::Lambert);

    }
    else
    {
        if (pieceType == "bishop")
            renderer->Render(rc, transform, black_bishop, ShaderId::Lambert);
        if (pieceType == "pawn")
            renderer->Render(rc, transform, black_pawn, ShaderId::Lambert);
        if (pieceType == "king")
            renderer->Render(rc, transform, black_king, ShaderId::Lambert);
        if (pieceType == "knight")
            renderer->Render(rc, transform, black_knight, ShaderId::Lambert);
        if (pieceType == "queen")
            renderer->Render(rc, transform, black_queen, ShaderId::Lambert);
        if (pieceType == "rook")
            renderer->Render(rc, transform, black_rook, ShaderId::Lambert);

    }

    // ---  体力バーの描画ロジック (選択時のみ) ---
    if (isSelected) {

        // 1. 体力比率を計算 (0.0 から 1.0)
        float healthRatio = (float)currentHealth / maxHealth;

        // 2. 体力バーの描画定数
        const float barWidth = 80.0f;
        const float barHeight = 8.0f;
        const float barYOffset = 60.0f; // 駒の上部に表示するためのオフセット

        // 3. 背景バー (最大体力) の描画
        DirectX::XMFLOAT4X4 bgTransform;
        DirectX::XMStoreFloat4x4(&bgTransform,
            DirectX::XMMatrixScaling(barWidth, barHeight, 1.0f) * // スケーリング
            DirectX::XMMatrixTranslation(
                boardPosition.x * 100.0f,
                barYOffset,
                boardPosition.y * 100.0f
            )
        );
        // renderer->Render(rc, bgTransform, healthBarBackground, ShaderId::UnlitRed); 

        // 4. 現在の体力バーの描画
        float currentBarWidth = barWidth * healthRatio;

        DirectX::XMFLOAT4X4 fillTransform;
        DirectX::XMStoreFloat4x4(&fillTransform,
            DirectX::XMMatrixScaling(currentBarWidth, barHeight, 1.0f) * DirectX::XMMatrixTranslation(
                // X軸のオフセットを計算 (左端を揃えるために、描画幅の半分だけ移動)
                boardPosition.x * 100.0f - (barWidth - currentBarWidth) / 2.0f,
                barYOffset,
                boardPosition.y * 100.0f
            )
        );
        //renderer->Render(rc, fillTransform, healthBarModel, ShaderId::UnlitGreen);

        // 暫定的な描画: モデルが未定義の場合、デバッグとして四角を描画するなどしてください。
        // モデルを描画する場合は、上記のコメントアウトを外し、適切な Model* と ShaderId を設定してください。
    }
   
}

void Piece::heal(int amount) {
    // 1. 現在の体力に回復量を加算
    currentHealth += amount;

    // 2. 最大体力 (maxHealth) を超えないように制限
    if (currentHealth > maxHealth) {
        currentHealth = maxHealth;
    }

    // 注: 体力バーの表示更新は、通常、UpdateやRenderの直前に SceneGame 側で 
    // setDisplayInfo を呼ぶことで処理されるため、ここでの呼び出しは不要です。
}

void Piece::SetBoardPosition(Position pos) {
    boardPosition = pos;

    startPos = worldPos;
    targetPos = BoardToWorld(pos);
    animTime = 0.0f;
    animDuration = 0.5f; 
    isMoving = true;

    moveEffect->Play({startPos.x*100.0f,100.0f,startPos.z*100.0f},50.0f);
}

Position Piece::GetBoardPosition() const {
    return boardPosition;
}

void Piece::setDisplayInfo(bool isSelected, int currentHealth, int maxHealth) {
    this->isSelected = isSelected;
    this->currentHealth = currentHealth;
    this->maxHealth = maxHealth > 0 ? maxHealth : 1; // 0割りを防ぐ
}