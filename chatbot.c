#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>

#define IRC_SERVER "10.0.1.172"
#define IRC_PORT 6667
#define NICK "bviba1062"
#define USERNAME "viba1062"
#define CHANNEL_1 "#Unix2026"
#define CHANNEL_2 "viba1062"

struct sembuf p = {0, -1, SEM_UNDO};
struct sembuf v = {0, 1, SEM_UNDO};

void ask_ollama(const char *prompt, char *response_out) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "curl -s http://localhost:11434/api/generate -d '{\"model\": \"tinyllama\", \"prompt\": \"%s\", \"stream\": false}' | grep -o '\"response\":\"[^\"]*\"' | cut -d'\"' -f4", prompt);

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        strcpy(response_out, "Error: Could not connect to Ollama.");
        return;
    }
    fgets(response_out, 1024, fp);
    pclose(fp);
}

void irc_worker(const char *channel, int semid, int *shm_ptr) {
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[2048];
    char msg[1024];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(IRC_PORT);
    inet_pton(AF_INET, IRC_SERVER, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(1);
    }

    sprintf(msg, "NICK %s\r\nUSER %s 0 * :AI Bot\r\n", NICK, USERNAME);
    send(sock, msg, strlen(msg), 0);
    sleep(2);
    sprintf(msg, "JOIN %s\r\n", channel);
    send(sock, msg, strlen(msg), 0);

    while (1) {
        int len = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (len <= 0) break;
        buffer[len] = '\0';

        if (strncmp(buffer, "PING", 4) == 0) {
            buffer[1] = 'O';
            send(sock, buffer, len, 0);
        }

        if (strstr(buffer, "PRIVMSG") && strstr(buffer, USERNAME) && !strstr(buffer, ":b")) {
            char *chat_msg = strchr(buffer + 1, ':');
            if (chat_msg) {
                chat_msg++; // Skip the ':'
                
                semop(semid, &p, 1);
                printf("[%s] Processing message: %s", channel, chat_msg);
                
                char ai_response[1024];
                ask_ollama(chat_msg, ai_response);
                
                sprintf(msg, "PRIVMSG %s :%s\r\n", channel, ai_response);
                send(sock, msg, strlen(msg), 0);
                
                (*shm_ptr)++; // Increment message counter in shared memory
                semop(semid, &v, 1);
            }
        }
    }
    close(sock);
}

int main() {
    // 1. Setup Shared Memory
    int shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    int *shm_ptr = (int *)shmat(shmid, NULL, 0);
    *shm_ptr = 0;

    // 2. Setup Semaphores
    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, 1);

    printf("Starting bot on channels %s and %s...\n", CHANNEL_1, CHANNEL_2);

    pid_t pid1 = fork();
    if (pid1 == 0) {
        irc_worker(CHANNEL_1, semid, shm_ptr);
        exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        irc_worker(CHANNEL_2, semid, shm_ptr);
        exit(0);
    }

    // Parent process waits for user to terminate
    printf("Bot running. Press Ctrl+C to stop.\n");
    wait(NULL);
    wait(NULL);

    // Cleanup
    shmdt(shm_ptr);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);

    return 0;
}
