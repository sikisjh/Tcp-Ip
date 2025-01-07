#include"..\Common.h"
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>

#define PORT 9000
#define MAX_CLIENTS 10
#define BUFSIZE 512

SOCKET clientSockets[MAX_CLIENTS]; //Ŭ���̾�Ʈ ��Ĺ�� ����
int numClients = 0; // �������� numClients ����, ������ Ŭ���̾�Ʈ ��

void handleNewClient(SOCKET newClientSocket) {
    const char* START_STRING = "ä�� ������ �����Ϸ��� ���͸� �Է��Ͻÿ�";
    if (numClients < MAX_CLIENTS) {
        clientSockets[numClients++] = newClientSocket; // ������ �����ϸ� Ŭ���̾�Ʈ �� 1����
        send(newClientSocket, START_STRING, strlen(START_STRING), 0);
        printf("%d��° ����� �߰�.\n", numClients);
    }
    else {
        printf("�ִ� Ŭ���̾�Ʈ ���� �ʰ��Ͽ� ������ �ź��մϴ�.\n");
        closesocket(newClientSocket);
    }
}

void aaa(void* sock) {
    SOCKET clientSocket = *(SOCKET*)sock; // Ŭ���̾�Ʈ ���� ������ ������
    char recvBuf[BUFSIZE];
    int bytesReceived;

    // Ŭ���̾�Ʈ ������� �˸�
    printf("Ŭ���̾�Ʈ�� ����Ǿ����ϴ�. ���� Ŭ���̾�Ʈ ��: %d\n", numClients);

    while (1) {
        bytesReceived = recv(clientSocket, recvBuf, BUFSIZE - 1, 0);
        if (bytesReceived > 0) {
            recvBuf[bytesReceived] = '\0';

            // ������ ���� �޽����� �ٸ� Ŭ���̾�Ʈ�鿡�� ���� (���� �κ�)
            char combinedMessage[BUFSIZE];                                          //�̸� + ���� ������
            snprintf(combinedMessage, sizeof(combinedMessage), "%s", recvBuf);      //combinedMessage�� recvBuf�� ����� ������ ���� ����

            for (int i = 0; i < numClients; ++i) {                                  //Ŭ���̾�Ʈ ��Ĺ�� �ϳ��� �����ϸ� ���� ���
                if (clientSockets[i] != clientSocket) {                             //clientSockets[i] Ŭ���̾�Ʈ ��Ĺ�� ����
                    send(clientSockets[i], combinedMessage, strlen(combinedMessage), 0);
                }
            }
            printf("%s\n", combinedMessage);
        }
        else {
            numClients--;  // ������ ������ ����� Ŭ���̾�Ʈ �� ����
            printf("Ŭ���̾�Ʈ���� ������ ������ϴ�. ���� Ŭ���̾�Ʈ ��: %d\n", numClients);
            closesocket(clientSocket); // Ŭ���̾�Ʈ ���� �ݱ�
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
        printf("Winsock �ʱ�ȭ ����\n");
        return 1;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        printf("���� ���� ����\n");
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); //ip �ּ�
    serverAddr.sin_port = htons(PORT);              //(tcp��ȣ)��Ĺ ��ȣ

    retval = bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (retval == SOCKET_ERROR) {
        printf("���ε� ����\n");
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    retval = listen(listenSocket, SOMAXCONN);
    if (retval == SOCKET_ERROR) {
        printf("��⿭ ���� ����\n");
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    while (1) {
        if (numClients < MAX_CLIENTS) {
            clientSocket = accept(listenSocket, NULL, NULL);
            if (clientSocket == INVALID_SOCKET) {
                printf("���� ���� ����\n");
                closesocket(listenSocket);
                WSACleanup();
                return 1;
            }

            handleNewClient(clientSocket);          //Ŭ���̾�Ʈ ���� ���� �Լ�

            // Ŭ���̾�Ʈ�� ���� ó���� ���� ������ ����
            HANDLE clientThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)aaa, (LPVOID)&clientSocket, 0, NULL);
            CloseHandle(clientThread);
        }
    }

    closesocket(listenSocket);
    WSACleanup();

    return 0;
}