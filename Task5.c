#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#define MAX_PATH 1024

int bin_width = 1024;
int max_bin = 100;
int histogram[100] = {0};

void get_directory_list(const char *directory) {
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

        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s/%s", directory, ent->d_name);

        struct stat st;
        if (stat(path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                get_directory_list(path);
            } else if (S_ISREG(st.st_mode)) {
                off_t size = st.st_size;
                int bin = size / bin_width;
                if (bin < max_bin) {
                    histogram[bin]++;
                } else {
                    histogram[max_bin - 1]++;
                }
            }
        }
    }

    closedir(dir);
}

void print_histogram() {
    printf("\nHistogram of File Sizes:\n");
    for (int i = 0; i < max_bin; i++) {
        if (histogram[i] > 0) {
            printf("Size %d - %d: %d files\n",
                   i * bin_width, (i + 1) * bin_width - 1, histogram[i]);
                   for(int j = 0 ; j < histogram[i]; j++){
                       printf("*");
                   }
                   printf("\n");
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <directory>\n", argv[0]);
        return 1;
    }

    get_directory_list(argv[1]);
    print_histogram();

    return 0;
}
