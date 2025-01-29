#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFSIZE 1024

int main() {
    int sockfd;
    char buffer[BUFSIZE];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;

    // יצירת סוקט
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // הגדרת כתובת השרת
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // קישור הסוקט לכתובת
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("UDP server listening on port %d...\n", PORT);

    while (1) {
        
        len = sizeof(cliaddr);

        // קבלת הודעה מהלקוח
        int n = recvfrom(sockfd, buffer, BUFSIZE, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0'; // סיום ההודעה

        // יצירת תהליך בן
        pid_t pid = fork();

        if (pid == 0) { // תהליך הבן - מטפל בבקשה
            printf("Received packet from %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
            printf("Client message: %s\n", buffer);

            // שליחת ההודעה חזרה (echo)
            sendto(sockfd, buffer, n, MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
            printf("Echoed message back to client.\n");

            // תהליך הבן מסיים את עבודתו
            close(sockfd);
            exit(0);
        } else if (pid < 0) {
            perror("fork failed");
        }

        // תהליך האב ממשיך להאזין להודעות נוספות
    }

    // סגירת הסוקט
    close(sockfd);
    return 0;
}
