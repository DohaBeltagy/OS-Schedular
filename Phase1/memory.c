#include <stdio.h>
#include <math.h>

// Size of vector of pairs
int size;

// Global vector of pairs to store
// address ranges available in free list
typedef struct
{
    int first;
    int second;
} Pair;

Pair free_list[100000][1000]; // Assuming a maximum of 1000 pairs in each list

// Map used as hash map to store the starting
// address as key and size of allocated segment
// key as value
typedef struct
{
    int key;
    int value;
} Map;

Map mp[100000]; // Assuming a maximum of 100000 elements in the map

void initialize(int sz)
{
    // Maximum number of powers of 2 possible
    int n = ceil(log(sz) / log(2));
    size = n + 1;

    for (int i = 0; i <= n; i++)
    {
        // Clear free_list
        for (int j = 0; j < 1000; j++)
        {
            free_list[i][j].first = -1; // Initialize to -1 indicating empty
            free_list[i][j].second = -1;
        }
    }

    // Initially whole block of specified
    // size is available
    free_list[n][0].first = 0;
    free_list[n][0].second = sz - 1;
}

int allocate(int sz) {
    // Calculate index in free list
    int n = ceil(log(sz) / log(2));

    // Check for a suitable block starting from the calculated index
    for (int i = n; i < size; i++) {
        printf("First: %d\n", free_list[i][0].first);
        printf("Second: %d\n", free_list[i][0].second);
        if (free_list[i][0].first != -1) {
            Pair temp = free_list[i][0];

            // Remove block from free list
            for (int j = 0; j < 999; j++) {
                free_list[i][j] = free_list[i][j + 1];
            }

            // Split larger blocks if necessary
            i--;
            for (; i >= n; i--) {
                Pair pair1, pair2;
                pair1.first = temp.first;
                pair1.second = temp.first + (temp.second - temp.first) / 2;
                pair2.first = temp.first + (temp.second - temp.first + 1) / 2;
                pair2.second = temp.second;

                // Add the two buddies to the free list at appropriate indices
                for (int j = 0; j < 1000; j++) {
                    if (free_list[i][j].first == -1) {
                        free_list[i][j] = pair1;
                        break;
                    }
                }
                for (int j = 0; j < 1000; j++) {
                    if (free_list[i][j].first == -1) {
                        free_list[i][j] = pair2;
                        break;
                    }
                }

                temp = free_list[i][0]; // Update temp for next iteration

                // Remove the first block for further splitting
                for (int j = 0; j < 999; j++) {
                    free_list[i][j] = free_list[i][j + 1];
                }
            }

            printf("Memory from %d to %d allocated\n", temp.first, temp.second);
            mp[temp.first].key = temp.second - temp.first + 1; // Store size for deallocation
            return temp.first; // Return the starting address of the allocated block
        }
    }

    // No suitable block found, allocation failed
    printf("Sorry, failed to allocate memory\n");
    return -1;
}

int checkAllocation(int sz)
{
    // Calculate index in free list
    // to search for block if available
    int n = ceil(log(sz) / log(2));

    printf("In check: %d\n", free_list[n][0].first);
    // Block available
    if (free_list[n][0].first != -1)
    {
        printf("First in check: %d\n", free_list[n][0].first);
        return 1;
    }
    else
    {
        int i;
        for (i = n + 1; i < size; i++)
        {
            // Find block size greater than request
            printf("First in check: %d\n", free_list[n][0].first);
            if (free_list[i][0].first != -1)
            {
                break;
            }
        }

        // If no such block is found
        // i.e., no memory block available
        if (i == size)
        {
            printf("yo\n");
            return 0;
        }
        else
        {
            return 1;
        }
    }
}

void deallocate(int start) {
    printf("in deallocate \n");
    printf("start = %d \n", start);

    // Find the size of memory block to deallocate
    int size = mp[start].key;
    printf("size = %d \n", size);

    // Calculate index in free list to search for proper position
    int index = ceil(log(size) / log(2));
    printf("index = %d \n", index);

    // Buddy merging logic
    int buddySize = size;
    while (buddySize <= size && index < size - 1) {
        int buddyAddress = start ^ buddySize; // Calculate buddy address
        int buddyFound = 0;
        for (int i = 0; i < 1000; i++) {
            if (free_list[index][i].first == buddyAddress && 
                free_list[index][i].second == buddyAddress + buddySize - 1) {
                buddyFound = 1;
                // Remove buddy from free_list
                for (int j = i; j < 999; j++) {
                    free_list[index][j] = free_list[index][j + 1];
                }
                break;
            }
        }

        if (buddyFound==1) {
            // Merge buddies
            start = fmin(start, buddyAddress);
            size *= 2;
            index++;
            buddySize *= 2;
        } else {
            break; // No buddy found, stop merging
        }
    }

    // Add the merged block (or original block if no merging) to free_list
    printf("Is it negative?: %d\n", free_list[index][2].first);
    for (int i = 0; i < 1000; i++) {
        printf("loop iteration = %d \n", i);
        printf("I don't know what is this gonna find out: %d\n", free_list[index][i].first);
        if (free_list[index][i].first == -1) {
            printf("free_list[index][i].first == -1 \n");
            free_list[index][i].first = start;
            free_list[index][i].second = start + size - 1;
            break;
        }
    }
    for (int i = 1; i < 1000 && free_list[index][i].first != -1; ++i) {
        Pair key = free_list[index][i];
        int j = i - 1;
        while (j >= 0 && free_list[index][j].first > key.first) {
            free_list[index][j + 1] = free_list[index][j];
            --j;
        }
        free_list[index][j + 1] = key;
    }

    printf("Memory from %d to %d deallocated\n", start, start + size - 1);
}