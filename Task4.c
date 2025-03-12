#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 5  
#define REF_COUNT 20 
#define PROBABILITY 0.7  

typedef struct {
    int page_number;
    unsigned char age_count;
} PageFrame;

void generate_page_references(int *references, int ref_count) {
    references[0] = rand() % N; 
    for (int i = 1; i < ref_count; i++) {
        if ((double)rand() / RAND_MAX < PROBABILITY) {
            references[i] = references[i - 1]; 
        } else {
            references[i] = rand() % N; 
        }
    }
}

void analyze_page_references(int *references, int ref_count) {
    int page_count[N] = {0};
    for (int i = 0; i < ref_count; i++) {
        page_count[references[i]]++;
    }

    printf("Page Frequency Distribution:\n");
    for (int i = 0; i < N; i++) {
        printf("Page %d: %d times\n", i, page_count[i]);
    }
}

int Local_aging_alg(int frame_count, int *page_references, int reference_count) {
    PageFrame *frames = (PageFrame *)malloc(frame_count * sizeof(PageFrame));
    for (int i = 0; i < frame_count; i++) {
        frames[i].page_number = -1;
        frames[i].age_count = 0;
    }

    int page_fault_count = 0;
    for (int i = 0; i < reference_count; i++) {
        int current_page = page_references[i];
        int found = 0;
        for (int j = 0; j < frame_count; j++) {
            if (frames[j].page_number == current_page) {
                frames[j].age_count |= 0x80; // Set the MSB to 1
                found = 1;
                break;
            }
        }
        if (!found) {
            page_fault_count++;
            int smallest_age = 0xFF;
            int smallest_age_index = 0;
            for (int k = 0; k < frame_count; k++) {
                if (frames[k].age_count < smallest_age) {
                    smallest_age = frames[k].age_count;
                    smallest_age_index = k;
                }
            }
            frames[smallest_age_index].page_number = current_page;
            frames[smallest_age_index].age_count = 0x80;
        }
        for (int k = 0; k < frame_count; k++) {
            frames[k].age_count >>= 1;
        }
    }
    printf("Total page faults: %d\n", page_fault_count);
    printf("Page Fault Rate: %.2f%%\n", (page_fault_count / (float)reference_count) * 100);
    free(frames);
    return page_fault_count;
}

int Global_aging_alg(int total_frames, int *references1, int *references2, int reference_count) {
    PageFrame *frames = (PageFrame *)malloc(total_frames * sizeof(PageFrame));
    for (int i = 0; i < total_frames; i++) {
        frames[i].page_number = -1;
        frames[i].age_count = 0;
    }

    int page_fault_count = 0;
    for (int i = 0; i < reference_count * 2; i++) {
        int current_page = (i % 2 == 0) ? references1[i / 2] : references2[i / 2]; 
        int found = 0;
        for (int j = 0; j < total_frames; j++) {
            if (frames[j].page_number == current_page) {
                frames[j].age_count |= 0x80;  
                found = 1;
                break;
            }
        }

        if (!found) {
            page_fault_count++;
            int smallest_age = 0xFF;
            int smallest_age_index = 0;
            for (int j = 0; j < total_frames; j++) {
                if (frames[j].age_count < smallest_age) {
                    smallest_age = frames[j].age_count;
                    smallest_age_index = j;
                }
            }
            frames[smallest_age_index].page_number = current_page;
            frames[smallest_age_index].age_count = 0x80;
        }

        for (int j = 0; j < total_frames; j++) {
            frames[j].age_count >>= 1;
        }
    }

    printf("Total Global Page Faults: %d\n", page_fault_count);
    printf("Global Page Fault Rate: %.2f%%\n", (page_fault_count / (float)(reference_count * 2)) * 100);
    free(frames);
    return page_fault_count;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <num_frames>\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    int frame_count = atoi(argv[1]);

    int references1[REF_COUNT], references2[REF_COUNT];
    generate_page_references(references1, REF_COUNT);
    generate_page_references(references2, REF_COUNT);

    printf("\nGenerated Page References for Process 1:\n");
    for (int i = 0; i < REF_COUNT; i++) {
        printf("%d ", references1[i]);
    }
    printf("\n");

    printf("\nGenerated Page References for Process 2:\n");
    for (int i = 0; i < REF_COUNT; i++) {
        printf("%d ", references2[i]);
    }
    printf("\n");

    printf("\nAnalyzing Page Reference Distribution for Process 1:\n");
    analyze_page_references(references1, REF_COUNT);

    printf("\nAnalyzing Page Reference Distribution for Process 2:\n");
    analyze_page_references(references2, REF_COUNT);
    
    printf("\nRunning Local Aging Algorithm...\n");
    int local_faults1 = Local_aging_alg(frame_count, references1, REF_COUNT);
    int local_faults2 = Local_aging_alg(frame_count, references2, REF_COUNT);
    int total_local_faults = local_faults1 + local_faults2;

    printf("\nRunning Global Aging Algorithm...\n");
    int global_faults = Global_aging_alg(frame_count * 2, references1, references2, REF_COUNT);
    return 0;
}
