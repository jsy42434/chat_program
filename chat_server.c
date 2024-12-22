#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    char username[50];
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// 모든 클라이언트에게 메시지를 방송
void broadcast_message(const char *message, int sender_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket != sender_socket) { // 보낸 사람 제외
            if (send(clients[i].socket, message, strlen(message), 0) < 0) {
                perror("Failed to send message");
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// 클라이언트를 처리하는 쓰레드
void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[BUFFER_SIZE];
    char username[50];
    char time_buffer[20];

    // 사용자 등록
    memset(username, 0, sizeof(username));
    if (recv(client_socket, username, sizeof(username), 0) <= 0) {
        perror("Failed to get username");
        close(client_socket);
        return NULL;
    }
    username[strcspn(username, "\r\n")] = '\0'; // 개행 문자 제거

    pthread_mutex_lock(&clients_mutex);
    clients[client_count].socket = client_socket;
    strncpy(clients[client_count].username, username, sizeof(username));
    client_count++;
    pthread_mutex_unlock(&clients_mutex);

    // 서버 출력: 사용자가 채팅방에 들어옴
    printf("[SERVER]: %s has joined the chat.\n", username);

    // 클라이언트에게 알림
    snprintf(buffer, BUFFER_SIZE, "[SERVER]: %s has joined the chat.\n", username);
    broadcast_message(buffer, client_socket);

    // 클라이언트와 통신
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            // 서버 출력: 사용자가 채팅방을 떠남
            printf("[SERVER]: %s has disconnected.\n", username);

            // 클라이언트에게 알림
            snprintf(buffer, BUFFER_SIZE, "[SERVER]: %s has left the chat.\n", username);
            broadcast_message(buffer, client_socket);

            // 클라이언트 목록에서 제거
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < client_count; i++) {
                if (clients[i].socket == client_socket) {
                    for (int j = i; j < client_count - 1; j++) {
                        clients[j] = clients[j + 1];
                    }
                    client_count--;
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            close(client_socket);
            return NULL;
        }
        buffer[bytes_received] = '\0';

        // 서버 출력: 사용자가 메시지를 보냄
        printf("[%s]: %s\n", username, buffer);

        // 메시지 방송
        char message_with_username[BUFFER_SIZE + 50];
        snprintf(message_with_username, sizeof(message_with_username), "[%s]: %.900s", username, buffer);
        broadcast_message(message_with_username, client_socket);
    }
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // 소켓 생성
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 소켓 바인딩
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // 연결 대기
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    printf("Chat server is running on port %d...\n", PORT);

    // 클라이언트 연결 처리
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Failed to accept client");
            continue;
        }

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, &client_socket) != 0) {
            perror("Failed to create thread");
            close(client_socket);
        }
    }

    close(server_socket);
    return 0;
}

