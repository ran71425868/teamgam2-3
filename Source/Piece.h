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

    /**
     * @brief 駒の色を取得する
     * @return 駒の色 ("white" または "black")
     */
    const std::string& getColor() const {
        return color;
    }

    // ★追加: 体力を取得するメソッド (logicPieceに処理を委譲)
    int getHealth() const {
        // ポインタが有効な場合のみ、体力を取得
        return (logicPiece) ? logicPiece->getHealth() : 0;
    }
    int getMaxHealth() const {
        return (logicPiece) ? logicPiece->getMaxHealth() : 0;
    }

    /**
     * @brief 駒のタイプを取得する
     * @return 駒のタイプ ("king", "pawn" など)
     */
    const std::string& getType() const {
        // 既存の private メンバー pieceType を使用
        return pieceType;
    }

    /**
     * @brief ダメージを適用する
     * @param damage 適用するダメージ量
     */
    void takeDamage(int damage) {
        currentHealth -= damage;
        if (currentHealth < 0) {
            currentHealth = 0;
        }
        // 注: setDisplayInfoを呼び出して、体力バーの表示を更新することも可能です。
        // setDisplayInfo(is
        // Selected, currentHealth, maxHealth); 
    }

    void heal(int amount); // ★この宣言を追加する！

    // 以前のロジックで必要とされたため、Position取得関数名も修正
    Position getPosition() const {
        return GetBoardPosition();
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

    // ★追加: 対応する論理駒 (ChessPiece) へのポインタ
    // BoardがChessPieceオブジェクトの生存期間を管理しているため、ここでは生ポインタを使用
    ChessPiece* logicPiece = nullptr;

    int board_count = 0;

    Effect* moveEffect = nullptr;

};
