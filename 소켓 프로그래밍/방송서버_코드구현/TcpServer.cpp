#include"..\Common.h"
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>

#define PORT 9000
#define MAX_CLIENTS 10
#define BUFSIZE 512

SOCKET clientSockets[MAX_CLIENTS]; //클라이언트 소캣을 저장
int numClients = 0; // 전역변수 numClients 선언, 접속한 클라이언트 수

void handleNewClient(SOCKET newClientSocket) {
    const char* START_STRING = "채팅 서버에 연결하러면 엔터를 입력하시오";
    if (numClients < MAX_CLIENTS) {
        clientSockets[numClients++] = newClientSocket; // 서버에 접속하면 클라이언트 수 1증가
        send(newClientSocket, START_STRING, strlen(START_STRING), 0);
        printf("%d번째 사용자 추가.\n", numClients);
    }
    else {
        printf("최대 클라이언트 수를 초과하여 연결을 거부합니다.\n");
        closesocket(newClientSocket);
    }
}

void aaa(void* sock) {
    SOCKET clientSocket = *(SOCKET*)sock; // 클라이언트 소켓 정보를 가져옴
    char recvBuf[BUFSIZE];
    int bytesReceived;

    // 클라이언트 연결됨을 알림
    printf("클라이언트가 연결되었습니다. 현재 클라이언트 수: %d\n", numClients);

    while (1) {
        bytesReceived = recv(clientSocket, recvBuf, BUFSIZE - 1, 0);
        if (bytesReceived > 0) {
            recvBuf[bytesReceived] = '\0';

            // 서버가 받은 메시지를 다른 클라이언트들에게 전송 (수정 부분)
            char combinedMessage[BUFSIZE];                                          //이름 + 받은 데이터
            snprintf(combinedMessage, sizeof(combinedMessage), "%s", recvBuf);      //combinedMessage에 recvBuf에 저장된 내용을 복사 저장

            for (int i = 0; i < numClients; ++i) {                                  //클라이언트 소캣을 하나씩 증가하며 각각 방송
                if (clientSockets[i] != clientSocket) {                             //clientSockets[i] 클라이언트 소캣을 저장
                    send(clientSockets[i], combinedMessage, strlen(combinedMessage), 0);
                }
            }
            printf("%s\n", combinedMessage);
        }
        else {
            numClients--;  // 서버와 연결이 끊기면 클라이언트 수 감소
            printf("클라이언트와의 연결이 끊겼습니다. 현재 클라이언트 수: %d\n", numClients);
            closesocket(clientSocket); // 클라이언트 소켓 닫기
            break;
        }
    }
}

int main() {
    WSADATA wsaData;
    SOCKET listenSocket, clientSocket;
    struct sockaddr_in serverAddr;
    int retval;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Winsock 초기화 실패\n");
        return 1;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        printf("소켓 생성 실패\n");
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); //ip 주소
    serverAddr.sin_port = htons(PORT);              //(tcp번호)소캣 번호

    retval = bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (retval == SOCKET_ERROR) {
        printf("바인딩 실패\n");
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    retval = listen(listenSocket, SOMAXCONN);
    if (retval == SOCKET_ERROR) {
        printf("대기열 설정 실패\n");
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    while (1) {
        if (numClients < MAX_CLIENTS) {
            clientSocket = accept(listenSocket, NULL, NULL);
            if (clientSocket == INVALID_SOCKET) {
                printf("연결 수락 실패\n");
                closesocket(listenSocket);
                WSACleanup();
                return 1;
            }

            handleNewClient(clientSocket);          //클라이언트 연결 관련 함수

            // 클라이언트에 대한 처리를 위한 스레드 생성
            HANDLE clientThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)aaa, (LPVOID)&clientSocket, 0, NULL);
            CloseHandle(clientThread);
        }
    }

    closesocket(listenSocket);
    WSACleanup();

    return 0;
}