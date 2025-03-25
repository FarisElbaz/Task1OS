#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

struct process{
    pid_t pid;
    int arrival;
    int burst;
    int waiting;
    int turnaround;
};

void input_process(struct process *p){
    printf("Enter process ID: ");
    scanf("%d", &p->pid);
    printf("Enter arrival time: ");
    scanf("%d", &p->arrival);
    printf("Enter burst time: ");
    scanf("%d", &p->burst);
}

int compare_fcfs(const void *a, const void *b) {
    struct process *p1 = (struct process *)a;
    struct process *p2 = (struct process *)b;
    return p1->arrival - p2->arrival;
}

int compare_sjf(const void *a, const void *b) {
    struct process *p1 = (struct process *)a;
    struct process *p2 = (struct process *)b;

    if (p1->arrival < p2->arrival)
        return -1;
    else if (p1->arrival > p2->arrival)
        return 1;
    return p1->burst - p2->burst;
}
void fcfs(struct process *processes, int n){
    qsort(processes,n,sizeof(struct process),compare_fcfs);
    int current_time = 0;
    for(int i=0; i < n; i++){
        if(current_time < processes[i].arrival){
            current_time = processes[i].arrival;
        }
        processes[i].waiting = current_time - processes[i].arrival;
        processes[i].turnaround = processes[i].waiting + processes[i].burst;
        current_time += processes[i].burst;
    }
}

void sjf(struct process *processes, int n) {
    qsort(processes, n, sizeof(struct process), compare_sjf);
    int current_time = 0;

    for (int i = 0; i < n; i++) {
        if (current_time < processes[i].arrival) {
            current_time = processes[i].arrival;
        }
        processes[i].waiting = current_time - processes[i].arrival;
        processes[i].turnaround = processes[i].waiting + processes[i].burst;
        current_time += processes[i].burst;
    }
}

void Round_robin(struct process *processes, int n, int quantum) {
    int remaining_burst[n];
    int current_time = 0;
    int done = 0;

    for (int i = 0; i < n; i++) {
        remaining_burst[i] = processes[i].burst;
    }

    while (done < n) {
        int executed = 0; 

        for (int i = 0; i < n; i++) {
            if (remaining_burst[i] > 0 && processes[i].arrival <= current_time) {
                executed = 1;

                if (remaining_burst[i] > quantum) {
                    current_time += quantum;
                    remaining_burst[i] -= quantum;
                } else {
                    current_time += remaining_burst[i];
                    processes[i].waiting = current_time - processes[i].arrival - processes[i].burst;
                    processes[i].turnaround = processes[i].waiting + processes[i].burst;
                    remaining_burst[i] = 0;
                    done++;
                }
            }
        }
        if (!executed) {
            current_time++;
        }
    }
}

int main() {
    int n, quantum;
    printf("Enter number of processes: ");
    scanf("%d", &n);
    
    struct process processes[n];
    for (int i = 0; i < n; i++) {
        input_process(&processes[i]);
    }

    printf("\nFirst Come First Serve (FCFS) Scheduling:\n");
    fcfs(processes, n);
    printf("Process\tWaiting Time\tTurnaround Time\n");
    for (int i = 0; i < n; i++) {
        printf("%d\t%d\t\t%d\n", processes[i].pid, processes[i].waiting, processes[i].turnaround);
    }

    printf("\nShortest Job First (SJF) Scheduling:\n");
    sjf(processes, n);
    printf("Process\tWaiting Time\tTurnaround Time\n");
    for (int i = 0; i < n; i++) {
        printf("%d\t%d\t\t%d\n", processes[i].pid, processes[i].waiting, processes[i].turnaround);
    }

    printf("Enter time quantum for Round Robin: ");
    scanf("%d", &quantum);


    printf("\nRound Robin Scheduling:\n");
    Round_robin(processes, n, quantum);
    printf("Process\tWaiting Time\tTurnaround Time\n");
    for (int i = 0; i < n; i++) {
        printf("%d\t%d\t\t%d\n", processes[i].pid, processes[i].waiting, processes[i].turnaround);
    }

    return 0;
}
