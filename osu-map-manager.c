#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <omp.h>
#include <pthread.h>

#define BUFFER_SIZE 1024
#define MAX_THREADS 16
#define SERVER_IP "199.36.253.252"
#define PORT_NO 443
#define CONTENT_SIZE 10000000

void print_help() {
    printf("Ez az osu map converter!\n\n");
    printf("- Ha szeretned a mapjaidat lementeni txt-be, akkor -m2t\n");
    printf("- Ha szeretned, mar meglévo txt mentesbol beimportalni a mapjaidat, akkor -t2m\n");
    printf("\nRoviden:\n- Map to Text: -m2t\n- Text to Map: -t2m\n\n");
}

char * getPath() {
    char* path;
    printf("Add meg az osu mappa eleresi utvonalat: ");
    scanf("%s", path);
    return path;
}


char ipv4[BUFFER_SIZE]() {
    char hostname[BUFFER_SIZE];
    char ipv4[BUFFER_SIZE];
    struct hostent *host_entry;
    struct in_addr **addr_list;

    // Lekéri a gép nevét
    if (gethostname(hostname, sizeof(hostname)) == -1) { fprintf(stderr, "Nem sikerült lekerni a gep nevet!\n"); exit(1); }

    printf("Gép neve: %s\n", hostname);

    // Lekéri a gép részletes információit
    host_entry = gethostbyname(hostname);
    if (host_entry == NULL) { fprintf(stderr, "Nem sikerult lekerni a gep informacioit!\n"); exit(1); }

    // Lekéri az IPv4 címet
    addr_list = (struct in_addr **)host_entry->h_addr_list;
    for (int i = 0; addr_list[i] != NULL; i++) {
        sprintf(ipv4, "%s", inet_ntoa(*addr_list[i]));
    }
    return ipv4;
}


void mapsToText(char* path) {
    DIR* dir;
    FILE *file;
    struct dirent *entry;
    char name[1024];
    char *deskpath;
    int length;

    deskpath = getenv("USERPROFILE");
    if (deskpath == NULL) { fprintf(stderr, "Hiba a felhasznalo konytaranak lekerdezesenel!\n"); exit(1); }
    strcat(deskpath,"\\Desktop\\maps.txt");

    // printf("### filepath: %s\n", deskpath);
    // printf("### dirpath: %s\n", path);

    file = fopen(deskpath, "w");
    if (file == NULL) { fprintf(stderr, "Fajl nem nyithato meg!\n"); exit(1);}

    dir = opendir(path);
    if (dir == NULL) { fprintf(stderr, "Hiba a mappa megnyitasa soran!\n"); exit(1); }

    while ((entry = readdir(dir)) != NULL) {
        // printf("\n''%s''   ", entry->d_name);
        if (isdigit(entry->d_name[0])) {
            char* space_pos = strstr(entry->d_name, " ");
            if (space_pos == NULL) { length = strlen(entry->d_name); }
            else                   { length = space_pos - entry->d_name; }
            strncpy(name, entry->d_name, length);
            name[length] = '\0';
            fprintf(file, "%s\n", name);
            // printf("### %s\n", name);
        }
    }
    closedir(dir);
    fclose(file);

    printf("\n\nmaps.txt letrehozva (%s)\n\n", deskpath);
}

void textToMaps(char* path) {
    FILE* file;
    int current, last;
    char line[50];

    char *deskpath = getenv("USERPROFILE");
    if (deskpath == NULL) { fprintf(stderr, "Hiba a felhasznalo konytaranak lekerdezesenel!\n"); exit(1); }
    strcat(deskpath,"\\Desktop\\maps.txt");

    file = fopen(deskpath, "r");
    if (file == NULL) { fprintf(stderr, "Fajl nem nyithato meg!\n"); exit(1);}

    int lineCount = 0;
    while (fscanf(file, "%d", &current) == 1) { lineCount++; }
    fclose(file);

    file = fopen(deskpath, "r");
    if (file == NULL) { fprintf(stderr, "Fajl nem nyithato meg!\n"); exit(1);}

    chdir(path);
    // printf("### pazh: %s\n",path);



    //"https://bm7.ppy.sh/d/%s?fs=%s%20Claire%20Rouge%20%28CV%3A%20Ibuki%20Kido%29%20-%20Blade%20Dance.osz&fd=2115723.osz&ts=1714776938&cs=0341843a21ef3c9eff7c65b74cee8004&nv=0"


    int sock;                        // Socket ID
    int bytes;                       // Elküldött/Fogadott bájtok
    int flag;                        // Átküldési flag
    char on;                         // sockopt beállítás
    unsigned int server_size;        // sockaddr_in szerver hossza
    struct sockaddr_in server;       // Szerver címe
    char request[CONTENT_SIZE];      // Szervernek küldött üzenet
    char response[CONTENT_SIZE];     // Szerver válasza

    on   = 1;
    flag = 0;
    server.sin_family      = AF_INET;
    // server.sin_addr.s_addr = inet_addr(SERVER_IP);
    inet_pton(AF_INET, SERVER_IP, &server.sin_addr);
    server.sin_port        = htons(PORT_NO);
    server_size            = sizeof server;

    // Socket létrehozása és beállítása
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { fprintf(stderr, "Hiba: A socketet nem sikerult letrehozni.\n"); exit(5); }
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == -1) {
        fprintf(stderr, "Kapcsolodas a szerverhez sikertelen");
        close(sockfd);
        exit(10);
    }

    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    char* current_time = ("%s, %s %s %s %s:%s:%s GMT",
        local_time->tm_wday, local_time->tm_mday, local_time->tm_mon, local_time->tm_year, local_time->tm_hour, local_time->tm_min, local_time->tm_sec);

    sprintf(response, "HTTP/1.1 200 OK\r\n"
                      "Server: nginx/1.23.3\r\n"
                      "Date: %s\r\n"            // Fri, 03 May 2024 22:22:16 GMT
                      "Content-Type: application/download\r\n"
                      "Content-Length: %d\r\n"  // 1542651
                      "Connection: keep-alive\r\n"
                      "Access-Control-Allow-Origin: https://collections.osustuff.ri.mk\r\n"
                      "Content-Disposition: attachment;filename=\"%s.osz\";\r\n"   // 2140349 xaev - the elon musk
                      "X-Frame-Options: SAMEORIGIN\r\n\r\\0"current_time, content_length, filename);

    if (send(sockfd, request, strlen(request), 0) == -1) {
        fprintf(stderr, "Keres kuldese sikertelen!\n");
        close(sockfd);
        exit(11);
    }

    char* link;
    sprintf(request, "GET %s HTTP/1.1\r\n"      //   /b/2140349/bUUSCmq1LXQZZmofGRzdWRRypOOwB3WV/
                      "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,/*;q=0.8\r\n"
                      "Accept-Encoding: gzip, deflate, br\r\n"
                      "Accept-Language: en-US,en;q=0.6\r\n"
                      "Connection: keep-alive\r\n"
                      "Host: beatconnect.io\r\n"
                      "Referer: https://beatconnect.io/\r\n"
                      "Sec-Fetch-Dest: document\r\n"
                      "Sec-Fetch-Mode: navigate\r\n"
                      "Sec-Fetch-Site: same-origin\r\n"
                      "Sec-Fetch-User: ?1\r\n"
                      "Sec-GPC: 1\r\n"
                      "Upgrade-Insecure-Requests: 1\r\n"
                      "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36\r\n"
                      "sec-ch-ua: \"Not A(Brand\";v=\"99\", \"Brave\";v=\"121\", \"Chromium\";v=\"121\"\r\n"
                      "sec-ch-ua-mobile: ?0\r\n"
                      "sec-ch-ua-platform: \"Windows\"\r\n\r\n\0", link);



    bytes = recv(sockfd, response, sizeof(response), 0);
    if (bytes == -1) { fprintf("Valasz fogadasa sikertelen!\n");
        close(sockfd);
        exit(12);
    }
    response[bytes_received] = '\0';

    printf("Szerver válasza:\n%s\n", response);

    close(sock);



    #pragma omp parallel for schedule(guided)
    for (int i = 0; i < lineCount; i++)
    {
        fscanf(file, "%d", &current);
        last = current;
        char command[1000];
        sprintf(command, "curl https://beatconnect.io/b/%d/ -o %d.osz\0", current, current);
        system(command);
        printf("letoltve: szal(%d)", omp_get_thread_num());
    }

    fclose(file);

    sprintf(line,"%d.osz\0", last);
    system(line);
}


int main(int argc, char* argv[])
{
    int mode = -1;
    if (argc != 2)           { print_help();  exit(1); }
    if (strcmp(argv[1], "-m2t") == 0)      { mode = 1; }
    else if (strcmp(argv[1], "-t2m") == 0) { mode = 0; }
    else                      { print_help(); exit(1); }

    char* path = getPath();

    if (mode == -1) { print_help(); exit(1); }
    else if (mode)  { printf("### m2t\n"); mapsToText(path); }
    else if (!mode) { printf("### t2m\n"); textToMaps(path); }

    return 0;
}
