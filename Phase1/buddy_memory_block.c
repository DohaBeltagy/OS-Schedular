// #include <stdio.h>
// #include <stdlib.h>
// #include <math.h>

// #define MEMORY_SIZE 1024
// #define MIN_BLOCK_SIZE 4

// // Structure to represent a block in the buddy system tree
// // typedef struct {
// //     int level;       // Level in the tree (0 for smallest blocks)
// //     int is_allocated; // 1 if allocated, 0 if free
// // } Block;

// typedef struct 
// {
//     int memory[1024];
// }Block;



// // Function to initialize the buddy system tree
// // void buddy_init(Block* tree) {
// //     // Initially, the entire memory is a single free block
// //     tree[0].level = 0;  // Level 0 corresponds to the largest block
// //     tree[0].is_allocated = 0;
// // }

// //Function to initialize the buddy system tree
// void buddy_init(Block* tree) {
//     for (int i = 0; i < 1024; i++)
//     {
//         tree->memory[i] = 0;
//     }
// }

// // Function to find the index of a buddy block
// // int find_buddy_index(int index, int level) {
// //     return index ^ (1 << level); // XOR operation to find the buddy
// // }

// // // Function to allocate memory using the buddy system
// // int buddy_alloc(int size, Block* tree) {
// //     // Find the level in the tree that corresponds to the requested size
// //     int level = 0;
// //     while ((MIN_BLOCK_SIZE << level) < size) {
// //         level++;
// //     }

// //     // Find the first available block at the required level or higher
// //     int index = 0;
// //     while (index < (((MEMORY_SIZE / MIN_BLOCK_SIZE) * 2) - 1)) {
// //         if (tree[index].level >= level && !tree[index].is_allocated) {
// //             // Found a suitable block
// //             tree[index].is_allocated = 1;

// //             // Split the block if it's larger than needed
// //             while (tree[index].level > level) {
// //                 int buddy_idx = find_buddy_index(index, tree[index].level - 1);
// //                 tree[buddy_idx].level = tree[index].level - 1;
// //                 tree[buddy_idx].is_allocated = 0;
// //                 tree[index].level--;
// //             }
// //             return index * MIN_BLOCK_SIZE; // Return the starting address of the allocated block
// //         }
// //         index++;
// //     }
// //     return -1; // Allocation failed
// // }

// int get_level(int size)
// {
//     int level = 10;
//     int i = 0;
//     while (i < 10)
//     {
//         if (pow(2, level) < size)
//         {
//             return level + 1;
//         }
//         else
//         {
//             level--;
//         }
//         i++;
//     }
// }

// int buddy_alloc_helper(int size, Block* tree, int start, int end, bool* found, int level)
// {
//     int counter = 0;
//     int index = -1;
//     bool allocated = false;
//     printf("from block slot: %d\n", tree->memory[256]);
//     for (int i = start; i < end; i++)
//     {
//         if (counter < pow(2, get_level(size)))
//         {
//             if (tree->memory[i] == 0)
//             {
//                 if (index == -1)
//                 {
//                     index = i;
//                 }
//                 counter++;
//             }
//             else
//             {
//                 counter = 0;
//                 index = -1;
//             }
//         }
//     }
//     if(counter == pow(2, get_level(size)))
//     {
//         allocated = true;
//     }
//     printf("counter: %d\n", counter);
//     if (counter == pow(2, get_level(size)))
//     {
//         allocated = true;
//     }
//     if (counter > 0 && !allocated)
//     {
//         int remaining = pow(2, level)- counter;
//         for (int i = end; i < end + remaining; i++)
//         {
//             if (counter < pow(2, get_level(size)))
//             {
//                 if (tree->memory[i] == 1)
//                 {
//                     break;
//                 }
//                 else
//                 {
//                     counter++;
//                 }
//             }
//         }
//         if(counter == pow(2, get_level(size)))
//         {
//             allocated = true;
//         }
//     }
//     if (allocated)
//     {
//         printf("Block: %d\n", (int)pow(2, level));
//         for (int i = index; i < pow(2, level) + index; i++)
//         {
//             tree->memory[i] = 1;
//         }
//         *found = true;
//         printf("Index: %d\n", index);
//         return index;
//     }
//     else
//     {
//         return -1;
//     }
// }

// int buddy_alloc(int size, Block* tree, int start, int end, bool* found, int level) {
//     // Find the level in the tree that corresponds to the requested size
//     if (start < end)
//     {
//         int l;
//         int r;
//         int ans;
//         int mid = start + (end - start) / 2;
//         if (((end - start) / 2 ) < size)
//         {
//             printf("My start: %d, and end: %d and I am gonna start\n", start, end);
//             ans = buddy_alloc_helper(size, tree, start, end, found, level);
//         }
//         else
//         {
//             printf("My start: %d, and end: %d left side\n", start, mid);
//             l = buddy_alloc(size, tree, start, mid, found, level - 1);
//             if (*found)
//             {
//                 return l;
//             }
//             printf("My start: %d, and end: %d right side\n", mid, end);
//             r = buddy_alloc(size, tree, mid, end, found, level - 1);
//             if (*found)
//             {
//                 return r;
//             }
//         }
//         if (*found)
//         {
//             return ans;
//         }
//         else
//         {
//             return -1;
//         }
//     }
// }

// // Function to free memory using the buddy system
// // void buddy_free(int address, Block* tree) {
// //     // Calculate the index of the block to be freed
// //     int index = address / MIN_BLOCK_SIZE;

// //     // Mark the block as free
// //     tree[index].is_allocated = 0;

// //     // Merge with buddy if possible
// //     int level = tree[index].level;
// //     while (level > 0) {
// //         int buddy_idx = find_buddy_index(index, level - 1);
// //         if (!tree[buddy_idx].is_allocated && tree[buddy_idx].level == level - 1) {
// //             // Buddy is free, merge the blocks
// //             tree[index].level++;
// //             index = (index < buddy_idx) ? index : buddy_idx;
// //         } else {
// //             break;
// //         }
// //         level--;
// //     }
// // }

// //Function to free memory using the buddy system
// void buddy_free(int address, Block* tree, int size) {
//     int block = pow(2, get_level(size));
//     printf("This is the block: % d\n", block);
//     for (int i = address; i < block + address; i++)
//     {
//         tree->memory[i] = 0;
//     }
// }