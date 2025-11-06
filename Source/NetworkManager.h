#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

// どの駒を動かしたか＋座標情報
struct MoveData {
    int pieceType; // 駒の種類（必要に応じて定義）
    int fromX, fromY;
    int toX, toY;
};

// UDP 通信管理クラス
class NetworkManager {
public:
    enum class Mode {
        Server,
        Client
    };

    NetworkManager();
    ~NetworkManager();

    bool Initialize(Mode mode, const char* ip, int port);
    void Finalize();

    void SendMove(const MoveData& move);
    bool ReceiveMove(MoveData& move);

private:
    Mode mode_;
    SOCKET sock_;
    sockaddr_in remoteAddr_;
};