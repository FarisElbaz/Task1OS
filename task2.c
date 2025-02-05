#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORD_LENGTH 100

typedef struct {
    char *segment;
    int segment_size;
    int *word_count;
    pthread_mutex_t *mutex;
} ThreadData;

void *count_words(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    char *segment = data->segment;
    int segment_size = data->segment_size;
    int *word_count = data->word_count;
    pthread_mutex_t *mutex = data->mutex;

    char word[MAX_WORD_LENGTH];
    int index = 0;
    for (int i = 0; i < segment_size; i++) {
        if (isalpha(segment[i])) {
            word[index++] = tolower(segment[i]);
        } else if (index > 0) {
            word[index] = '\0';
            pthread_mutex_lock(mutex);
            word_count[word[0] - 'a']++;
            pthread_mutex_unlock(mutex);
            index = 0;
        }
    }
    if (index > 0) {
        word[index] = '\0';
        pthread_mutex_lock(mutex);
        word_count[word[0] - 'a']++;
        pthread_mutex_unlock(mutex);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <num_threads>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];
    int num_threads = atoi(argv[2]);

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *text = malloc(file_size + 1);
    fread(text, 1, file_size, file);
    text[file_size] = '\0';
    fclose(file);

    int segment_size = file_size / num_threads;
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];
    int word_count[26] = {0};
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    int start = 0;
    for (int i = 0; i < num_threads; i++) {
        int end = start + segment_size;
        if (i == num_threads - 1) {
            end = file_size;
        } else {
            // Adjust end to avoid splitting words
            while (end < file_size && isalpha(text[end])) {
                end++;
            }
        }

        thread_data[i].segment = text + start;
        thread_data[i].segment_size = end - start;
        thread_data[i].word_count = word_count;
        thread_data[i].mutex = &mutex;
        pthread_create(&threads[i], NULL, count_words, &thread_data[i]);

        start = end;
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    free(text);

    printf("Word frequency count:\n");
    for (int i = 0; i < 26; i++) {
        if (word_count[i] > 0) {
            printf("%c: %d\n", 'a' + i, word_count[i]);
        }
    }

    return 0;
}