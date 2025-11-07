#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "NetworkManager.h"

NetworkManager::NetworkManager()
    : mode_(Mode::Server), sock_(INVALID_SOCKET)
{
}

NetworkManager::~NetworkManager()
{
    Finalize();
}

bool NetworkManager::Initialize(Mode mode, const char* ip, int port)
{
    mode_ = mode;

    // WinSock 初期化
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return false;

    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_ == INVALID_SOCKET)
        return false;

    // ノンブロッキングモード
    u_long modeFlag = 1;
    ioctlsocket(sock_, FIONBIO, &modeFlag);

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = (mode == Mode::Server) ? INADDR_ANY : inet_addr(ip);

    if (mode == Mode::Server) {
        if (bind(sock_, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
            return false;
    }
    else {
        memcpy(&remoteAddr_, &addr, sizeof(addr));
    }

    return true;
}

void NetworkManager::Finalize()
{
    if (sock_ != INVALID_SOCKET) {
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
    }
    WSACleanup();
}

void NetworkManager::SendMove(const MoveData& move)
{
    sendto(sock_, (const char*)&move, sizeof(move), 0,
        (sockaddr*)&remoteAddr_, sizeof(remoteAddr_));
}

bool NetworkManager::ReceiveMove(MoveData& move)
{
    sockaddr_in fromAddr;
    int fromLen = sizeof(fromAddr);
    int result = recvfrom(sock_, (char*)&move, sizeof(move), 0,
        (sockaddr*)&fromAddr, &fromLen);

    if (result == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK)
            return false; // データなし
        return false;
    }

    if (mode_ == Mode::Server) {
        memcpy(&remoteAddr_, &fromAddr, sizeof(fromAddr));
    }

    return (result > 0);
}
