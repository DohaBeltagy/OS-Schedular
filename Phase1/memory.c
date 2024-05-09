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

int allocate(int sz)
{
    // Calculate index in free list
    // to search for block if available
    int n = ceil(log(sz) / log(2));

    // Block available
    if (free_list[n][0].first != -1)
    {
        Pair temp = free_list[n][0];

        // Remove block from free list
        for (int i = 0; i < 999; i++)
        {
            free_list[n][i] = free_list[n][i + 1];
        }
        printf("Memory from %d to %d allocated\n", temp.first, temp.second);
        // map starting address with
        // size to make deallocating easy
        mp[temp.first].key = temp.second - temp.first + 1;
        return temp.first;
    }
    else
    {
        int i;
        for (i = n + 1; i < size; i++)
        {
            // Find block size greater than request
            if (free_list[i][0].first != -1)
            {
                break;
            }
        }

        // If no such block is found
        // i.e., no memory block available
        if (i == size)
        {
            printf("Sorry, failed to allocate memory\n");
            return -1;
        }
        else
        {
            Pair temp = free_list[i][0];

            // Remove first block to split it into halves
            for (int j = 0; j < 999; j++)
            {
                free_list[i][j] = free_list[i][j + 1];
            }
            i--;

            for (; i >= n; i--)
            {
                // Divide block into two halves
                Pair pair1, pair2;
                pair1.first = temp.first;
                pair1.second = temp.first + (temp.second - temp.first) / 2;
                pair2.first = temp.first + (temp.second - temp.first + 1) / 2;
                pair2.second = temp.second;
                for (int j = 0; j < 1000; j++)
                {
                    if (free_list[i][j].first == -1)
                    {
                        free_list[i][j] = pair1;
                        break;
                    }
                }
                for (int j = 0; j < 1000; j++)
                {
                    if (free_list[i][j].first == -1)
                    {
                        free_list[i][j] = pair2;
                        break;
                    }
                }
                temp = free_list[i][0];

                // Remove first free block to
                // further split
                for (int j = 0; j < 999; j++)
                {
                    free_list[i][j] = free_list[i][j + 1];
                }
            }
            printf("Memory from %d to %d allocated\n", temp.first, temp.second);
            mp[temp.first].key = temp.second - temp.first + 1;
            return temp.first;
        }
    }
}

int checkAllocation(int sz)
{
     // Calculate index in free list
    // to search for block if available
    int n = ceil(log(sz) / log(2));

    // Block available
    if (free_list[n][0].first != -1)
    {
       return 1;
    }
    else
    {
        int i;
        for (i = n + 1; i < size; i++)
        {
            // Find block size greater than request
            if (free_list[i][0].first != -1)
            {
                break;
            }
        }

        // If no such block is found
        // i.e., no memory block available
        if (i == size)
        {
            return 0;
        }
        else
        {
           return 1;
        }
    }
}

void deallocate(int start)
{
    printf("in deallocate \n");
    printf("size = %d \n", start);
    // Find the size of memory block to deallocate
    int size = mp[start].key;
printf("size = %d \n", size);
    // Calculate index in free list
    // to search for proper position
    int index = ceil(log(size) / log(2));
printf("index = %d \n", index);
    // Add the memory block to free list
    for (int i = 0; i < 1000; i++)
    {
        printf("loop iteration = %d \n", i);
        if (free_list[index][i].first == -1)
        {
            printf("free_list[index][i].first == -1 \n");
            free_list[index][i].first = start;
            free_list[index][i].second = start + size - 1;
            break;
        }
    }

    printf("Memory from %d to %d deallocated\n", start, start + size - 1);
}