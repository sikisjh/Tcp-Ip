#include"..\Common.h"
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>

#define SERVER_IP "192.168.159.118 " // ���� IP �ּ� ��Ʈ��:192.168.216.118,  ����:192.168.123.100 , �б��ڸ�: 203.247.198.120
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
            printf("�������� ������ ������ϴ�.\n");
            break;
        }
        else {
            printf("���� ����\n");
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
        printf("Winsock �ʱ�ȭ ����\n");
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("���� ���� ����\n");
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_port = htons(PORT);

    retval = connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (retval == SOCKET_ERROR) {
        printf("������ ������ �� �����ϴ�.\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // ������ ���� ������ ����
    HANDLE recvThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)receiveMessages, (LPVOID)sock, 0, NULL);

    char sendBuf[BUFSIZE];
    char sendBuf2[BUFSIZE];
    char name[50];

    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0; // ���� ���� ����
    printf("--ä�ü��� ����--\n");

    printf("����� �̸� �Է�:");
    fgets(sendBuf2, BUFSIZE, stdin);
    sendBuf2[strcspn(sendBuf2, "\n")] = '\0';

    while (1) {
        printf("�޽��� �Է�: ");
        fgets(sendBuf, BUFSIZE, stdin);
        sendBuf[strcspn(sendBuf, "\n")] = '\0';
        

        // �̸��� �޽��� ����
        char combinedMessage[BUFSIZE];
        snprintf(combinedMessage, sizeof(combinedMessage), "\n(%s) : %s", sendBuf2, sendBuf);

        // ������ ����
        if (send(sock, combinedMessage, strlen(combinedMessage), 0) == SOCKET_ERROR) {
            printf("�۽� ����\n");
            break;
        }
    }


    WaitForSingleObject(recvThread, INFINITE);
    CloseHandle(recvThread);

    closesocket(sock);
    WSACleanup();

    return 0;
}