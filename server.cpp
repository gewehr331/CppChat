#include <iostream>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <thread>
#include <vector>
#include <fstream>
#include <string>
#include <windows.h>

#pragma comment(lib, "ws2_32")

#define SERVER_PORT "9000"

class Connection {
private:
    std::string login;
    std::string password;

public:
    Connection(std::string login_, std::string password_) {
        login = login_;
        password = password_;
    }

    Connection() {
        login="";
        password="";
    }



    bool CheckCreds() {
        std::string res_user_creds = login + ":" + password;
        std::string line;
        std::fstream file;
        file.open("user_list.txt");
        if (file.is_open()) {
            while(std::getline(file, line)) {
                if (line==res_user_creds) {
                    file.close();
                    return true;
                }
            }
        }
        file.close();
        return false;
    }

    void SendMes(std::string to_user, std::string message) {
        if (CheckCreds()) {
            std::fstream file;
            file.open("chats/" + this->login + "-" + to_user + ".txt");
            if (file.is_open()) {
                file << "User:" << this->login << ":" << message << "\n";
            }
            file.close();
        }
    }

    void GetLoginFromClient(SOCKET& ClientSocket) {
        char recv_buf[512];
        int recv_buf_len = 512;

        int iResult = recv(ClientSocket, recv_buf, recv_buf_len, 0);

        if (iResult < 0) {
            std::cout << "Error of recv" << iResult << std::endl;
            closesocket(ClientSocket);
            return;
        } else if (iResult==0) {
            std::cout << "Closing connection" << std::endl;
            closesocket(ClientSocket);
            return;
        } else {

            this->login = recv_buf;
            std::cout << "Login of client is: " << this->login << std::endl;
        }

    }

    void GetPasswordFromClient(SOCKET& ClientSocket) {
            char recv_buf[512];
            int recv_buf_len = 512;

            int iResult = recv(ClientSocket, recv_buf, recv_buf_len, 0);

            if (iResult < 0) {
                std::cout << "Error of recv" << std::endl;
                closesocket(ClientSocket);
                return;
            } else if (iResult==0) {
                std::cout << "Closing connection" << std::endl;
                closesocket(ClientSocket);
                return;
            } else {
                this->password = recv_buf;
                std::cout << "Password of client is: " << this->password << std::endl;
            }

        }

    bool ParseMessageFromClient(SOCKET& ClientSocket) {

        char recv_buf[512];
        int recv_buf_len = 512;

        int iResult = recv(ClientSocket, recv_buf, recv_buf_len, 0);

        if (iResult < 0) {
            std::cout << "Error of recv" << std::endl;
            closesocket(ClientSocket);
            return false;
        } else if (iResult==0) {
            std::cout << "Closing connection" << std::endl;
            closesocket(ClientSocket);
            return false;
        } else {
            std::string message = recv_buf;
            size_t del = message.find(':');
            if (del==std::string::npos) {
                send(ClientSocket, recv_buf, recv_buf_len,0);
                std::cout << "Not correct message" << std::endl;
                closesocket(ClientSocket);
                return false;
            }
            std::string command = message.substr(0, del);

            if (command == "send") {
                std::string UserData = message.substr(del+1, message.length()-del-1);
                size_t del2 = UserData.find(':');
                if (del2==std::string::npos) {
                    send(ClientSocket, recv_buf, recv_buf_len,0);
                    std::cout << "Not correct message" << std::endl;
                    closesocket(ClientSocket);
                    return false;
                }

                std::string to_user = UserData.substr(0, del2);
                std::string data = UserData.substr(del2+1, to_user.length()-del2-1);

                SendMes(to_user, data);
                return true;
            }
            else if (command=="get_chat_history") {
                std::string to_user = message.substr(del+1, message.length()-del-1);
                std::string messages = this->SendChatWith(to_user);
                messages = messages + "\n";
                send(ClientSocket, messages.c_str(), 512,0);
                return true;
            }

        }
        return false;
    }

    std::string SendChatWith(std::string user) {
        std::fstream file;
        std::string messages;
        file.open("chats/" + login + "-" + user + ".txt");
        if (file.is_open()) {
            while(!file.eof()) {
                std::string buf;
                std::getline(file, buf);
                messages = messages + buf;
            }
        }
        return messages;
    }
};


class ChatServer {
private:
    SOCKET ListenSocket;
    struct addrinfo *result;
    int num_of_conns;

    void chat() {
        SOCKET ClientSocket = INVALID_SOCKET;
        char recv_buf[512];
        int recv_buf_len = 512;
        ClientSocket = accept(this->ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            std::cout << "Error of accepting:" << WSAGetLastError() << std::endl;
        }

        int iResult = 0;
        std::string login;
        std::string password;

        Connection c;
        //Получение логина с клиента
        c.GetLoginFromClient(ClientSocket);

        //Получение пароля с клиента
        c.GetPasswordFromClient(ClientSocket);

        if (c.CheckCreds()==false) {
            send(ClientSocket, "Auth error", 512, 0);
            closesocket(ClientSocket);
            return;
        } else {
            send(ClientSocket, "Auth success", 512, 0);
        }

        // Получение сообщения
        bool res_get_mes = true;
        while (res_get_mes) {
            res_get_mes = c.ParseMessageFromClient(ClientSocket);
        }
        closesocket(ClientSocket);
        return;
    }

public:

    ChatServer() {
        WSADATA WsaData;
        int iResult = 0;
        iResult = WSAStartup(MAKEWORD(2,2), &WsaData);

        if (iResult!=0) {
            std::cout << "Error of WsaStartUp:" << iResult << std::endl;
        }

        struct addrinfo *ptr = NULL, hints;
        this->result = NULL;

        ZeroMemory(&hints, sizeof(hints));

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_IP;
        hints.ai_flags = AI_PASSIVE;

        iResult = getaddrinfo(NULL, SERVER_PORT, &hints, &result);

        if (iResult!=0) {
            std::cout << "Error of getaddrinfo:" << iResult << std::endl;
            WSACleanup();
        }

        this->ListenSocket = INVALID_SOCKET;
        this->ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

        if (this->ListenSocket==INVALID_SOCKET) {
            std::cout << "ListenSocket is invalid" << std::endl;
            freeaddrinfo(result);
            WSACleanup();
        }
    }

    bool Bind() {
        int iResult = bind(this->ListenSocket, this->result->ai_addr, (int)this->result->ai_addrlen);
        if (iResult==SOCKET_ERROR) {
            std::cout << "Error of bind:" << WSAGetLastError() << std::endl;
            freeaddrinfo(this->result);
            closesocket(this->ListenSocket);
            WSACleanup();
            return false;
        }

        freeaddrinfo(this->result);
        return true;
    }

    bool Listen(int num_of_conns_) {
        this->num_of_conns = num_of_conns_;
        if (listen(this->ListenSocket, this->num_of_conns) == SOCKET_ERROR) {
            std::cout << "Error of listen:" << WSAGetLastError() << std::endl;
            closesocket(this->ListenSocket);
            WSACleanup();
            return false;
        }
        return true;
    }

    void Accept() {
        std::vector<std::thread> threads;
        for (int i = 0; i < (this->num_of_conns); i++) {
            threads.push_back(std::thread(&ChatServer::chat, this));
        }
        getchar();
    }

    ~ChatServer() {
        closesocket(this->ListenSocket);
        WSACleanup();
    }
};

int main() {
    std::vector<std::thread> threads;

    ChatServer server;
    if (!server.Bind()) {
        return 1;
    }
    if (!server.Listen(10)) {
        return 1;
    }
    server.Accept();
    int a;
    std::cin >> a;
    return 0;
}
