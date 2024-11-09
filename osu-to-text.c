#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

char * getLocation();
char * createFile();
void mapsTotext(char path[100], char deskpath[100]);

FILE *file;

int main(){
    char *path = getLocation();

    char *deskpath = createFile();
    
    if (strcmp(deskpath, "f") != 0) {
        //printf("deskpath != null \n");
        mapsTotext(path, deskpath);
    } else {
        printf("Nem sikerült\n");
    }


    free(deskpath);
    free(path);
    return 0;
}

char * getLocation() {
    char* path;

    printf("Adja meg az eleresi utat az osu mappadhoz: ");
    scanf("%s", path);

    return path;
}

char * createFile() {
    char *deskpath = getenv("USERPROFILE");

    if (deskpath != NULL) {
        strcat(deskpath, "\\Desktop\\maps.txt");
        printf("Asztal eleresi utvonal: %s\n", deskpath);

        file = fopen(deskpath, "w");

        if (file != NULL) {
            //printf("File != null \n");
            fprintf(file, "Hello!");
            fclose(file);
        } else {
            printf("Hiba a fajl letrehozasa soran.\n");
            strcpy(deskpath, "f");
        }
        
    } else {
        printf("Hiba az asztal eleresi utjanak lekerese soran.\n");
        strcpy(deskpath, "f");
    }
    return deskpath;
}

void mapsTotext(char *path, char *deskpath) {
    
    file = fopen(deskpath, "w");
    //fprintf(file, "Siker!");

    DIR *directory;
    struct dirent *entry;

    printf("Asztal eleresi utvonal: %s\n", path);
    directory = opendir(path);
    if (directory == NULL) {
        printf("Hiba a mappa megnyitasa soran. (opeindir hiba)\n");
        fclose(file);
        return;
    }

    while ((entry = readdir(directory)) != NULL) {
        // Csak mappa-e
        if (entry->d_type == DT_DIR) {
            if (isdigit(entry->d_name[0]) == 0) {
                //char *pos = strchr(entry->d_name, ' '); // Ellenőrizzük, hogy van-e szóköz a mappanévben
                //*pos = '\0'; // Ha van szóköz, levágjuk az utána következő részt
                char * v = strtok(entry->d_name, " ");
                fprintf(file, "%s\n", v[0]);
            }
        }
    }

    closedir(directory);
    fclose(file);
    readAndSortNumbersFromFile(deskpath);
    
}

int compare(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

void readAndSortNumbersFromFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Hiba a fájl megnyitásakor");
        exit(EXIT_FAILURE);
    }

    // Dinamikus tömb létrehozása a számok tárolásához
    int *numbers = NULL;
    int size = 0;
    int capacity = 10;

    numbers = (int*)malloc(capacity * sizeof(int));
    if (numbers == NULL) {
        perror("Memóriafoglalási hiba");
        exit(EXIT_FAILURE);
    }

    // Számok beolvasása a fájlból
    int number;
    while (fscanf(file, "%d", &number) == 1) {
        if (size == capacity) {
            capacity *= 2;
            numbers = realloc(numbers, capacity * sizeof(int));
            if (numbers == NULL) {
                perror("Memóriafoglalási hiba");
                exit(EXIT_FAILURE);
            }
        }
        numbers[size++] = number;
    }

     // Fájl bezárása
    fclose(file);

    // Számok rendezése
    qsort(numbers, size, sizeof(int), compare);

    // Fájl újra megnyitása kiírásra
    file = fopen(filename, "w");
    if (file == NULL) {
        perror("Hiba a fájl megnyitásakor");
        exit(EXIT_FAILURE);
    }

    // Rendezett számok kiírása a fájlba
    for (int i = 0; i < size; i++) {
        fprintf(file, "%d\n", numbers[i]);
    }

    // Fájl bezárása
    fclose(file);
    // Dinamikusan foglalt memória felszabadítása
    free(numbers);
}