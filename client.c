#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1" // 서버 IP
#define PORT 8080             // 서버 포트
#define BUFFER_SIZE 1024      // 버퍼 크기

void *receive_messages(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("Disconnected from server.\n");
            close(sock);
            exit(0);
        }
        printf("%s", buffer); // 서버에서 받은 메시지 출력
    }
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    pthread_t recv_thread;
    char username[50];

    // 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // 서버에 연결
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server.\n");

    // 사용자 이름 입력
    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0'; // 개행 문자 제거
    send(sock, username, strlen(username), 0);

    // 메시지 수신 쓰레드 시작
    if (pthread_create(&recv_thread, NULL, receive_messages, &sock) != 0) {
        perror("Thread creation failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 메시지 송신 루프
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        fgets(buffer, sizeof(buffer), stdin);

        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Exiting...\n");
            break;
        }

        // 서버로 메시지 전송
        send(sock, buffer, strlen(buffer), 0);
    }

    close(sock);
    return 0;
}
