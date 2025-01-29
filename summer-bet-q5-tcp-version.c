#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFSIZE 1024

int main() {
    int sockfd, clientfd;
    char buffer[BUFSIZE];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);

    // יצירת סוקט TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // הגדרת כתובת השרת
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // קשירת הסוקט לכתובת
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // השרת מאזין לחיבורים נכנסים
    listen(sockfd, 5);
    printf("TCP server listening on port %d...\n", PORT);

    while (1) {
        // קבלת חיבור נכנס
        clientfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);
        if (clientfd < 0) {
            perror("Accept failed");
            continue;
        }

        // יצירת תהליך בן לטיפול בלקוח
        pid_t pid = fork();
        if (pid == 0) { // תהליך הבן
            close(sockfd); // תהליך הבן לא צריך להאזין לחיבורים חדשים

            // קבלת הודעה מהלקוח
            int n = recv(clientfd, buffer, BUFSIZE, 0);
            if (n > 0) {
                buffer[n] = '\0'; // סימון סוף מחרוזת
                printf("Received packet from %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
                printf("Client message: %s\n", buffer);

                // שליחת ה-Echo חזרה
                send(clientfd, buffer, n, 0);
                printf("Echoed message back to client.\n");
            }

            close(clientfd); // סגירת החיבור עם הלקוח
            exit(0);
        } else if (pid < 0) {
            perror("Fork failed");
        }

        // תהליך האב ממשיך להאזין לחיבורים נוספים
        close(clientfd);
    }

    close(sockfd);
    return 0;
}
