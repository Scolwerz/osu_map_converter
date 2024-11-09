#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <string.h>
#include <dirent.h>

char * getLocation();
void textTomaps(char path[100], char filepath[100]);

FILE *file;

int main(){
    char *path = getLocation();
    
    char *deskpath = getenv("USERPROFILE");
    strcat(deskpath, "\\Desktop\\maps.txt");
    char *filepath = deskpath;

    //printf("%s",filepath);
    textTomaps(path, filepath);

    return 0;
}

char * getLocation() {
    char* path;

    printf("Add meg az eleresi utat, hogy hova mentse: ");
    scanf("%s", path);

    return path;
}



void textTomaps(char *path, char *deskpath) {
    chdir(path); // Munkakönyvtár megváltoztatása a megadott elérési útvonalra

    // Fájl megnyitása olvasásra
    FILE *file = fopen(deskpath, "r");
    if (file == NULL) {
        perror("Hiba a fájl megnyitásakor");
        return;
    }

    int a;
    int lastMap; // Buffer a sorok olvasásához
    while (fscanf(file,"%d",&a) == 1) {
        // A curl parancs futtatása a kapott adattal
        char command[1000]; // Buffer a curl parancs összeállításához
        sprintf(command ,"curl https://beatconnect.io/b/%d/ -o %d.osz", a, a);
        //printf("%s", command); // Ellenőrzésként kiírjuk a parancsot
        system(command); // curl parancs végrehajtása
        lastMap = a;
    }
    char line[50];
    sprintf(line,"%d.osz", lastMap);
    system(line);

    fclose(file); // Fájl bezárása
}