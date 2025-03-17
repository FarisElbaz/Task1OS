#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

void listfiles(const char*directory){
    DIR *dir = opendir(directory);
    if (dir == NULL) {
        printf("Error opening directory: %s\n", directory);
        return;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }
        char path[1000];
        snprintf(path, sizeof(path), "%s/%s", directory, ent->d_name);
        struct stat st;
        if (stat(path, &st) != 0) {
            perror("stat");
            continue;
        }
        if (S_ISDIR(st.st_mode)) {
                continue;
        }
        else if (S_ISREG(st.st_mode)) {
            off_t size = st.st_size;
            printf("%s\n", ent->d_name);
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    listfiles(argv[1]);

    return EXIT_SUCCESS;
}