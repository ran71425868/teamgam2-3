#pragma once
#include "System/ModelRenderer.h"
#include "Board.h"
#include "Effect.h"

class Piece {
public:
    Piece(std::string color, Position boardPos, std::string pieceType);
    ~Piece();

    void Update(float elapsedTime);
    void Render(const RenderContext& rc, ModelRenderer* renderer);

    void SetBoardPosition(Position pos);
    Position GetBoardPosition() const;
    
   
    /**@brief 駒の移動不可状態を設定する
       * @param immobilized trueで移動不可、falseで移動可能
       */
    void setImmobilized(bool immobilized) {
        IsImmobilized = immobilized;
    }

      

    /**
     * @brief 駒が移動不可状態であるかを取得する
     * @return trueなら移動不可
     */
    bool isImmobilized() const {
        return IsImmobilized;
    }

    void setDisplayInfo(bool isSelected, int currentHealth, int maxHealth);

    DirectX::XMFLOAT3 BoardToWorld(Position p) {
        return DirectX::XMFLOAT3(p.x, 0.0f, p.y);
    }
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

    //線形補完

    DirectX::XMFLOAT3 worldPos;
    DirectX::XMFLOAT3 startPos;
    DirectX::XMFLOAT3 targetPos;
    float animTime = 0.0f;
    float animDuration = 0.0f;     
    bool isMoving = false;

    // ★追加: 駒が移動不可状態であるかを示すフラグ
    bool IsImmobilized = false;

    int board_count = 0;

    Effect* moveEffect = nullptr;

};
