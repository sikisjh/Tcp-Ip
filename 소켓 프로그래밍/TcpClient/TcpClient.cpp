#include"..\Common.h"
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>

#define SERVER_IP "192.168.159.118 " // 서버 IP 주소 노트북:192.168.216.118,  집컴:192.168.123.100 , 학교자리: 203.247.198.120
#define PORT 9000
#define BUFSIZE 512

void receiveMessages(SOCKET sock) {
    char buf[BUFSIZE];
    int bytesReceived;

    while (1) {
        bytesReceived = recv(sock, buf, BUFSIZE - 1, 0);
        if (bytesReceived > 0) {
            buf[bytesReceived] = '\0';
            printf("%s\n", buf);
        }
        else if (bytesReceived == 0) {
            printf("서버와의 연결이 끊겼습니다.\n");
            break;
        }
        else {
            printf("수신 오류\n");
            break;
        }
    }
}

int main() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in serverAddr;
    int retval;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Winsock 초기화 실패\n");
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("소켓 생성 실패\n");
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_port = htons(PORT);

    retval = connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (retval == SOCKET_ERROR) {
        printf("서버에 연결할 수 없습니다.\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // 수신을 위한 스레드 생성
    HANDLE recvThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)receiveMessages, (LPVOID)sock, 0, NULL);

    char sendBuf[BUFSIZE];
    char sendBuf2[BUFSIZE];
    char name[50];

    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0; // 개행 문자 제거
    printf("--채팅서버 연결--\n");

    printf("사용할 이름 입력:");
    fgets(sendBuf2, BUFSIZE, stdin);
    sendBuf2[strcspn(sendBuf2, "\n")] = '\0';

    while (1) {
        printf("메시지 입력: ");
        fgets(sendBuf, BUFSIZE, stdin);
        sendBuf[strcspn(sendBuf, "\n")] = '\0';
        

        // 이름과 메시지 결합
        char combinedMessage[BUFSIZE];
        snprintf(combinedMessage, sizeof(combinedMessage), "\n(%s) : %s", sendBuf2, sendBuf);

        // 서버로 전송
        if (send(sock, combinedMessage, strlen(combinedMessage), 0) == SOCKET_ERROR) {
            printf("송신 오류\n");
            break;
        }
    }


    WaitForSingleObject(recvThread, INFINITE);
    CloseHandle(recvThread);

    closesocket(sock);
    WSACleanup();

    return 0;
}