#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

#define THRESHOLD (1 << 4) // Sort arrays of size 16

// Function to merge two sorted arrays
void merge(int arr[], int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;

    int L[n1], R[n2];
    for (int i = 0; i < n1; i++) {
        L[i] = arr[l + i];
    }
    for (int j = 0; j < n2; j++) {
        R[j] = arr[m + 1 + j];
    }

    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

// Function to sort an array using merge sort algorithm
void merge_sort(int arr[], int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        merge_sort(arr, l, m);
        merge_sort(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

int main(int argc, char *argv[]) {
    int pid1, status, pid2;
    int half = atoi(argv[1]); // 0 for left half, 1 for right half

    // Attach shared memory segment to process address space
    int shm_id = shmget(IPC_PRIVATE, 0, 0666);
    int *shm_ptr = shmat(shm_id, NULL, 0);

    // Get size of shared memory segment
    struct shmid_ds shmid_ds;
    shmctl(shm_id, IPC_STAT, &shmid_ds);
    int shm_size = shmid_ds.shm_segsz;

    // Divide shared memory segment in half
    int mid = shm_size / sizeof(int) / 2;
    int *left = &shm_ptr[half * mid];
    int *right = &shm_ptr[mid + half * mid];

    // Recurse until threshold is reached
    if (mid >= THRESHOLD) {
        // Create two child processes to sort the left and right halves
        if ((pid1 = fork()) == 0) {
            // Left half
            char *args[] = {"./cons", "0", NULL};
            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else if ((pid2 = fork()) == 0) {
           // Wait for child processes to complete
int status;
waitpid(pid1, &status, 0);
waitpid(pid2, &status, 0);

// Merge sorted left and right halves
merge(left, 0, mid - 1, mid + mid - 1);
merge_sort(left, 0, mid + mid - 1);

// Detach shared memory segment
shmdt(shm_ptr);

return 0;


