//
// Created by Саша on 28.09.2024.
//

#include <iostream>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <windows.h>
#include <vector>


#define SERVER_PORT "9000"
#define SERVER_NAME "localhost"
/*
SOCKET ClientInitialization() {
    WSADATA WsaData;
    int iResult = 0;
    iResult = WSAStartup(MAKEWORD(2,2), &WsaData);

    if (iResult!=0) {
        std::cout << "Error of WsaStartUp:" << iResult << std::endl;
        return INVALID_SOCKET;
    }

    struct addrinfo *result=NULL, *ptr=NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(SERVER_NAME, SERVER_PORT, &hints, &result);

    if (iResult!=0) {
        std::cout << "Error of getaddrinfo" << std::endl;
        WSACleanup();
        return INVALID_SOCKET;
    }

    SOCKET ClientSocket = INVALID_SOCKET;

    ptr = result;

    ClientSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

    if (ClientSocket == INVALID_SOCKET) {
        std::cout << "Error of creating Socket:" << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
    }

    iResult = connect(ClientSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

    if (iResult == SOCKET_ERROR) {
        closesocket(ClientSocket);
        ClientSocket = INVALID_SOCKET;
    }

    freeaddrinfo(result);

    if (ClientSocket==INVALID_SOCKET) {
        std::cout << "Error of connecting to server" << std::endl;
        WSACleanup();
        return INVALID_SOCKET;
    }

    return ClientSocket;
}*/

class ChatClient {
private:
    SOCKET ClientSocket;
    bool is_auth;
    std::string login;
public:

    ChatClient() {

        is_auth = false;


        WSADATA WsaData;
        int iResult = 0;
        iResult = WSAStartup(MAKEWORD(2,2), &WsaData);

        if (iResult!=0) {
            std::cout << "Error of WsaStartUp:" << iResult << std::endl;
        }

        struct addrinfo *result=NULL, *ptr=NULL, hints;
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        iResult = getaddrinfo(SERVER_NAME, SERVER_PORT, &hints, &result);

        if (iResult!=0) {
            std::cout << "Error of getaddrinfo" << std::endl;
            WSACleanup();
        }

        this->ClientSocket = INVALID_SOCKET;

        ptr = result;

        this->ClientSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (this->ClientSocket == INVALID_SOCKET) {
            std::cout << "Error of creating Socket:" << WSAGetLastError() << std::endl;
            freeaddrinfo(result);
            WSACleanup();
        }

        iResult = connect(this->ClientSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

        if (iResult == SOCKET_ERROR) {
            closesocket(this->ClientSocket);
            this->ClientSocket = INVALID_SOCKET;
        }

        freeaddrinfo(result);

        if (this->ClientSocket==INVALID_SOCKET) {
            std::cout << "Error of connecting to server" << std::endl;
            WSACleanup();
        }

    }

    bool Authenticate(std::string login_, std::string password) {

        int iResult = 0;
        char recv_buf[512];
        login = login_;

        iResult = send(ClientSocket, login.c_str(), 512, 0);
        iResult = send(ClientSocket, password.c_str(), 512, 0);

        iResult = recv(ClientSocket, recv_buf, 512, 0);

        if (recv_buf == "Auth error") {
            std::cerr << "Login or password is incorrect!!" << std::endl;
            this->is_auth = false;
        } else {
            std::cerr << "Auth is correct" << std::endl;
            this->is_auth = true;
        }

        return this->is_auth;

    }

    void SendMes(std::string to_user, std::string message) {
        int iResult = 0;
        std::string ResMes = "send:" + to_user + ":" + message;
        iResult = send(ClientSocket, ResMes.c_str(), 512, 0);
    }
    std::string GetChatHistory(std::string user) {

        int iResult = 0;
        char recv_buf[512];

        std::string ResMes = "get_chat_history:" + user;
        iResult = send(ClientSocket, ResMes.c_str(), 512, 0);
        iResult = recv(ClientSocket, recv_buf, 512, 0);
        return recv_buf;
    }

    ~ChatClient() {
        closesocket(ClientSocket);
        WSACleanup();
    }
};
int main() {

    ChatClient client;

    int iResult = 0;
    char recv_buf[512];
    std::string login_str;
    std::string password_str;
    std::cout << "Enter Login:" << std::endl;
    std::cin >> login_str;
    std::cout << "Enter Password:" << std::endl;
    std::cin >> password_str;

    if (!client.Authenticate(login_str, password_str)) {
        return 1;
    }

    std::string other_user;

    std::cout << "Get chat with:";
    std::cin >> other_user;

    std::cout << client.GetChatHistory(other_user);

    std::string message;

    std::cout << "Enter your message: ";
    std::cin >> message;
    client.SendMes(other_user, message);

    std::cout << client.GetChatHistory(other_user);



}
