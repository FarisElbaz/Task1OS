#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORD_LENGTH 100

typedef struct WordCount {
    char word[MAX_WORD_LENGTH];
    int count;
    struct WordCount *next;
} WordCount;

typedef struct {
    char *segment;
    int segment_size;
    WordCount **word_counts;
    pthread_mutex_t *mutex;
} ThreadData;

void *count_words(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    char *segment = data->segment;
    int segment_size = data->segment_size;
    WordCount **word_counts = data->word_counts;
    pthread_mutex_t *mutex = data->mutex;

    char word[MAX_WORD_LENGTH];
    int index = 0;

    for (int i = 0; i < segment_size; i++) {
        if (isalpha(segment[i])) {
            word[index++] = tolower(segment[i]);
        } else if (index > 0) {
            word[index] = '\0';
            pthread_mutex_lock(mutex);
            WordCount *wc = *word_counts;
            while (wc != NULL && strcmp(wc->word, word) != 0) {
                wc = wc->next;
            }
            if (wc == NULL) {
                wc = (WordCount *)malloc(sizeof(WordCount));
                strcpy(wc->word, word);
                wc->count = 1;
                wc->next = *word_counts;
                *word_counts = wc;
            } else {
                wc->count++;
            }
            pthread_mutex_unlock(mutex);
            index = 0;
        }
    }

    if (index > 0) {
        word[index] = '\0';
        pthread_mutex_lock(mutex);
        WordCount *wc = *word_counts;
        while (wc != NULL && strcmp(wc->word, word) != 0) {
            wc = wc->next;
        }
        if (wc == NULL) {
            wc = (WordCount *)malloc(sizeof(WordCount));
            strcpy(wc->word, word);
            wc->count = 1;
            wc->next = *word_counts;
            *word_counts = wc;
        } else {
            wc->count++;
        }
        pthread_mutex_unlock(mutex);
    }

    return NULL;
}

void adjust_segment_boundaries(char *text, int *start, int *end, long file_size) {
    // Move start backwards if it’s in the middle of a word
    while (*start > 0 && isalpha(text[*start])) {
        (*start)--;
    }
    // Move end forward if it’s in the middle of a word
    while (*end < file_size && isalpha(text[*end])) {
        (*end)++;
    }
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

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];
    WordCount *word_counts = NULL;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    int segment_size = file_size / num_threads;

    for (int i = 0; i < num_threads; i++) {
        int start = i * segment_size;
        int end = (i == num_threads - 1) ? file_size : start + segment_size;

        adjust_segment_boundaries(text, &start, &end, file_size);
        
        thread_data[i].segment = text + start;
        thread_data[i].segment_size = end - start;
        thread_data[i].word_counts = &word_counts;
        thread_data[i].mutex = &mutex;

        pthread_create(&threads[i], NULL, count_words, &thread_data[i]);

        printf("Thread %d: Start=%d, End=%d\n", i, start, end);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    WordCount *wc = word_counts;
    while (wc != NULL) {
        printf("%s: %d\n", wc->word, wc->count);
        WordCount *tmp = wc;
        wc = wc->next;
        free(tmp);
    }

    free(text);
    pthread_mutex_destroy(&mutex);

    return 0;
}
