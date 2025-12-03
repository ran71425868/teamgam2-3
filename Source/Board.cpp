#include "Board.h"
#include "King.h"
#include "Rook.h"
#include "Bishop.h"
#include "Knight.h"
#include "Queen.h"
#include "Pawn.h"
#include <iostream>
#include <iomanip>

Board::Board() {
    grid.resize(8, std::vector<std::shared_ptr<ChessPiece>>(8, nullptr));
    currentTurn = "white";

    black_bord = new Model("Data/Model/Stage/black_bord.mdl");
    white_bord = new Model("Data/Model/Stage/white_bord.mdl");
}

void Board::initialize() {
    // 白の駒
    grid[0][0] = std::make_shared<Rook>("white", Position{ 0, 0 });
    grid[0][1] = std::make_shared<Knight>("white", Position{ 1, 0 });
    grid[0][2] = std::make_shared<Bishop>("white", Position{ 2, 0 });
    grid[0][3] = std::make_shared<Queen>("white", Position{ 3, 0 });
    grid[0][4] = std::make_shared<King>("white", Position{ 4, 0 });
    grid[0][5] = std::make_shared<Bishop>("white", Position{ 5, 0 });
    grid[0][6] = std::make_shared<Knight>("white", Position{ 6, 0 });
    grid[0][7] = std::make_shared<Rook>("white", Position{ 7, 0 });
    for (int i = 0; i < 8; ++i)
        grid[1][i] = std::make_shared<Pawn>("white", Position{ i, 1 });

    // 黒の駒
    grid[7][0] = std::make_shared<Rook>("black", Position{ 0, 7 });
    grid[7][1] = std::make_shared<Knight>("black", Position{ 1, 7 });
    grid[7][2] = std::make_shared<Bishop>("black", Position{ 2, 7 });
    grid[7][3] = std::make_shared<Queen>("black", Position{ 3, 7 });
    grid[7][4] = std::make_shared<King>("black", Position{ 4, 7 });
    grid[7][5] = std::make_shared<Bishop>("black", Position{ 5, 7 });
    grid[7][6] = std::make_shared<Knight>("black", Position{ 6, 7 });
    grid[7][7] = std::make_shared<Rook>("black", Position{ 7, 7 });
    for (int i = 0; i < 8; ++i)
        grid[6][i] = std::make_shared<Pawn>("black", Position{ i, 6 });
}

void Board::printBoard() const {
    for (int y = 7; y >= 0; --y) {
        std::cout << y + 1 << " ";
        for (int x = 0; x < 8; ++x) {
            auto piece = grid[y][x];
            if (piece) {
                char symbol = piece->getType()[0];
                if (piece->getColor() == "black") symbol = tolower(symbol);
                std::cout << symbol << " ";
            }
            else {
                std::cout << ". ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "  a b c d e f g h\n";
}

std::shared_ptr<ChessPiece> Board::getPieceAt(Position pos) const {
    if (!isInsideBoard(pos)) return nullptr;

    auto piace = grid[pos.y][pos.x];
    return piace;
}

void Board::setPieceAt(Position pos, std::shared_ptr<ChessPiece> piece) {
    if (isInsideBoard(pos)) {
        grid[pos.y][pos.x] = piece;
        /*if (piece) piece->setPosition(pos);*/
    }
}

void Board::movePiece(Position from, Position to) {
    if (!isInsideBoard(from) || !isInsideBoard(to)) return;
    auto piece = grid[from.y][from.x];
    if (piece) {
        piece->setPosition(to);
        grid[to.y][to.x] = piece;
        grid[from.y][from.x] = nullptr;
    }
}

bool Board::isInsideBoard(const Position& pos) const {
    // 無効値チェック（未選択や異常値）
    if (pos.x < 0 || pos.y < 0) return false;

    // 上限チェック（盤面サイズに依存）
    constexpr int boardWidth = 8;
    constexpr int boardHeight = 8;

    if (pos.x >= boardWidth || pos.y >= boardHeight) return false;

    return pos.isValid();

}

// ターン管理
std::string Board::getCurrentTurn() const {
    return currentTurn;
}

void Board::switchTurn() {
    currentTurn = (currentTurn == "white") ? "black" : "white";
}

bool Board::isKingInCheck(std::string color) const {
    Position kingPos{ -1, -1 };
    bool kingFound = false;
    // 1. キングの位置を探す
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            auto piece = grid[y][x];
            if (piece && piece->getType() == "King" && piece->getColor() == color) {
                kingPos = { x, y };
                kingFound = true; // フラグを立てる
                break;           // 内部ループを抜ける
            }
        }
        if (kingFound) break; // フラグが立っていれば外部ループも抜ける
    }
    if (kingPos.x == -1) return false;

    // 2. 敵駒の合法手にキングが含まれるか確認 (再帰防止のためフラグを使用)
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            auto piece = grid[y][x];
            if (piece && piece->getColor() != color) {
                // isForCheck=true を渡す
                auto moves = piece->getLegalMoves(*this, true);

                for (auto& m : moves) {
                    if (m.x == kingPos.x && m.y == kingPos.y) {
                        return true; // キングが攻撃を受けている
                    }
                }
            }
        }
    }
    return false;
}

bool Board::isCheckmate(std::string color) const { // const を付け、盤面を書き換えないようにする

    // 1. まずチェックを受けているか確認
    // isKingInCheck は既に無限ループ対策（isForCheckフラグ）が施されている前提
    if (!isKingInCheck(color)) return false;

    // 2. 自軍のすべての駒の合法手を試して、チェックを回避できるか確認
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            auto piece = grid[y][x];

            // 自分の駒であるか確認
            if (piece && piece->getColor() == color) {

                // ここで getLegalMoves を呼び出すと、その内部で filterPinnedMoves が
                // isKingInCheck を呼び出すため、無限再帰の可能性があります。
                // 駒の基本的な移動先リスト（未フィルタリング）を取得するべきです。
                // ただし、ここでは既存の getLegalMoves を利用するため、
                // filterPinnedMoves の中で isKingInCheck を呼ばない設計が理想です。

                // 一旦、フィルタリングされた合法手を取得します
                auto moves = piece->getLegalMoves(*this);

                Position originalPos = { x, y };

                // 全ての合法な移動先について試す
                for (auto& m : moves) {

                    // --- 仮移動のシミュレーション（盤面のコピーを利用） ---

                    // 盤面を一時コピー
                    Board tempBoard = *this;

                    // 移動元の駒を取得 (tempBoard内から)
                    auto self = tempBoard.getPieceAt(originalPos);

                    // 駒を移動
                    tempBoard.setPieceAt(m, self);
                    tempBoard.setPieceAt(originalPos, nullptr);

                    // 駒の内部座標を更新 (この行がないとシミュレーションが破綻する可能性あり)
                    // if (self) self->setPosition(m); 

                    // 3. 移動後にチェックされていないか確認
                    // isKingInCheck(color) は、再帰を防止する isForCheck=true の動作をする必要があります。
                    // 盤面のコピーを使うため、元の盤面は変更されません。
                    bool stillInCheck = tempBoard.isKingInCheck(color);

                    // --- 元に戻す処理は不要（tempBoardが破棄されるため） ---

                    // チェック状態を回避できた手があれば、チェックメイトではない
                    if (!stillInCheck) return false;
                }
            }
        }
    }

    // どの駒のどの移動でもチェックを回避できなかった場合、チェックメイトである
    return true;
}

bool Board::isKingPresent(std::string color) const
{
    // 8x8のボード全体を走査
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {

            // 現在のマスにある駒を取得 (grid[y][x] が駒のポインタだと仮定)
            auto piece = grid[y][x];

            // 1. 駒が存在するか (nullptrでないか)
            if (piece) {
                // 2. その駒のタイプが "King" であるか
                bool isKing = (piece->getType() == "King");

                // 3. その駒の色が探している色と一致するか
                bool isTargetColor = (piece->getColor() == color);

                // 駒が存在し、キングであり、色が一致すれば、キングは生き残っている
                if (isKing && isTargetColor) {
                    return true;
                }
            }
        }
    }

    // ボード全体を走査してもキングが見つからなかった場合
    return false;
}

void Board::filterPinnedMoves(const ChessPiece& piece, std::vector<Position>& moves) const {
   
    std::vector<Position> legal;
    Position original = piece.getPosition();

    for (auto& m : moves) {
      
        Board temp = *this;

        auto moving = temp.getPieceAt(original);
        if (!moving) continue;

        auto captured = temp.getPieceAt(m);

        temp.setPieceAt(m, moving);
        temp.setPieceAt(original, nullptr);

        bool safe = !temp.isKingInCheck(piece.getColor());

        if (safe) legal.push_back(m);
    }

    moves = legal;
}

void Board::Render(const RenderContext& rc, ModelRenderer* renderer)
{
    for (int i = 0; i <= 7; i++)
    {
        for (int j = 0; j <= 7; j++)
        {
            DirectX::XMFLOAT4X4 bord_transform;
            DirectX::XMStoreFloat4x4(&bord_transform,
                DirectX::XMMatrixTranslation(
                    i * 100.0f,
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