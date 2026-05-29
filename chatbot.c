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

#define SERVER_IP "10.0.1.172"
#define PORT 6667
#define BOT_NICK "bviba1062"
#define USERNAME "viba1062"

int shmid, semid;
int *msg_count;

struct sembuf p = {0, -1, SEM_UNDO}; 
struct sembuf v = {0, 1, SEM_UNDO};  

void handle_sigint(int sig) {
    shmdt(msg_count);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    exit(0);
}

void ask_ollama(const char *prompt, char *response) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), 
        "curl -s http://localhost:11434/api/generate -d '{\"model\": \"tinyllama\", \"prompt\": \"%s\", \"stream\": false}' | grep -o '\"response\":\"[^\"]*\"' | cut -d'\"' -f4", 
        prompt);

    FILE *fp = popen(cmd, "r");
    if (fp) {
        if(fgets(response, 1024, fp) == NULL) {
            strcpy(response, "Error getting response.");
        }
        pclose(fp);
    }
}

void run_bot(const char *channel, int semid, int *shared_counter, int pipe_fd) {
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[2048], msg[1024];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        exit(1);
    }

    sprintf(msg, "NICK %s\r\nUSER %s 0 * :AI Bot\r\n", BOT_NICK, USERNAME);
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
            continue;
        }

        if (strstr(buffer, "PRIVMSG") && strstr(buffer, USERNAME) && !strstr(buffer, ":b")) {
            char *chat_content = strrchr(buffer, ':');
            if (chat_content) {
                chat_content++; 
                
                char ai_out[1024] = {0};
                ask_ollama(chat_content, ai_out);

                char log_msg[256];
                snprintf(log_msg, 256, "Channel %s: Replied to %s\n", channel, USERNAME);
                write(pipe_fd, log_msg, strlen(log_msg));

                semop(semid, &p, 1);
                (*shared_counter)++;
                semop(semid, &v, 1);

                sprintf(msg, "PRIVMSG %s :%s\r\n", channel, ai_out);
                send(sock, msg, strlen(msg), 0);
            }
        }
    }
    close(sock);
}

int main() {
    signal(SIGINT, handle_sigint);

    shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    msg_count = (int *)shmat(shmid, NULL, 0);
    *msg_count = 0;

    semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, 1);

    int fd[2];
    pipe(fd);

    if (fork() == 0) {
        close(fd[0]);
        run_bot("#Unix2026", semid, msg_count, fd[1]);
        exit(0);
    }

    if (fork() == 0) {
        close(fd[0]);
        run_bot(USERNAME, semid, msg_count, fd[1]);
        exit(0);
    }

    close(fd[1]);
    char log_buffer[256];
    while (1) {
        ssize_t bytes = read(fd[0], log_buffer, sizeof(log_buffer) - 1);
        if (bytes > 0) {
            log_buffer[bytes] = '\0';
            printf("%s", log_buffer);
            printf("Total messages: %d\n", *msg_count);
        }
    }

    return 0;
}
