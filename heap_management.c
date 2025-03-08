#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <limits.h> // Required for INT_MAX

#define HEAP_SIZE 1024
#define HEADER_SIZE sizeof(Block)
#define FOOTER_SIZE sizeof(size_t)
#define MAX_ALLOCATIONS 10 // Maximum number of tracked allocations for freeing

static char heap[HEAP_SIZE];

// Block structure: header holds the total block size, free flag
typedef struct Block {
    size_t size;
    int free;
    struct Block* next;
   // struct Block* buddy;  For Buddy System (not used in Fit strategies but kept for potential expansion)
} Block;

// Allocation Strategy enum
typedef enum {
    FIRST_FIT,
    BEST_FIT,
    WORST_FIT
} Strategy;

Strategy currStrategy = FIRST_FIT; // Default strategy

static char* strategyNames[] = {"First Fit", "Best Fit", "Worst Fit"};

// Function prototypes
Block* get_prev_block(Block* block);
Block* find_free_block(size_t total_size, Strategy strategy);
void* allocate_block(size_t size);
void split_block(Block* block, size_t total_size);
void merge_block(Block* block);
void free_block(void* ptr);
void init_heap();
void print_heap();
void set_footer(Block* block);

// Global pointer to the start of the heap
static Block* heap_head = NULL;
static void* allocated_pointers[MAX_ALLOCATIONS] = {NULL}; // Track allocated pointers for freeing
static int allocation_count = 0;

// Utility: Set the footer value for a block
void set_footer(Block* block) {
    size_t* footer = (size_t*)((char*)block + block->size - FOOTER_SIZE);
    *footer = block->size;
}

void init_heap() {
    heap_head = (Block*)heap;
    heap_head->size = HEAP_SIZE;
    heap_head->free = 1;
    heap_head->next = NULL;
  //  heap_head->buddy = NULL;  Initialize buddy pointer
    set_footer(heap_head);
}

void print_heap() {
    Block* curr = heap_head;
    printf("Heap status (Strategy: %s):\n", strategyNames[currStrategy]);
    while ((char*)curr < heap + HEAP_SIZE) {
        printf("Block at %p: size = %zu, free = %d", (void*)curr, curr->size, curr->free);
        if (curr->next != NULL) {
            printf(", next = %p", (void*)curr->next);
        }
        printf("\n");
        if (curr->next == NULL) break;
        curr = curr->next;
    }
}

// Find a free block based on the selected allocation strategy.
Block* find_free_block(size_t total_size, Strategy strategy) {
    Block* curr = heap_head;
    Block* best_block = NULL;
    Block* worst_block = NULL;
    size_t min_remainder = INT_MAX; // Initialize to maximum possible size_t value
    size_t max_size = 0;

    while ((char*)curr < heap + HEAP_SIZE) {
        if (curr->free && curr->size >= total_size) {
            if (strategy == FIRST_FIT) {
                return curr; // First fit: return immediately
            } else if (strategy == BEST_FIT) {
                if (!best_block) { // Initialize min_remainder only when BEST_FIT and for the first suitable block
                    min_remainder = INT_MAX; // Initialize min_remainder here for BEST_FIT
                }
                size_t remainder = curr->size - total_size;
                if (remainder < min_remainder) {
                    min_remainder = remainder;
                    best_block = curr;
                }
            } else if (strategy == WORST_FIT) {
                if (curr->size > max_size) {
                    max_size = curr->size;
                    worst_block = curr;
                }
            }
        }
        if (curr->next == NULL) break;
        curr = curr->next;
    }

    if (strategy == BEST_FIT) {
        return best_block;
    } else if (strategy == WORST_FIT) {
        if (worst_block && worst_block->size >= total_size) {
             return worst_block;
        } else {
            return NULL; // No worst-fit block large enough
        }
    }

    return NULL; // Should only reach here if strategy is FIRST_FIT and no block found or if BEST/WORST failed to find a suitable block
}


void split_block(Block* block, size_t total_size) {
    if (block->size >= total_size + HEADER_SIZE + FOOTER_SIZE + 1) {
        Block* newBlock = (Blo ck*)((char*)block + total_size);
        newBlock->size = block->size - total_size;
        newBlock->free = 1;
        newBlock->next = block->next;
       // newBlock->buddy = NULL;  Initialize buddy pointer
        block->size = total_size;
        block->free = 0;
        block->next = newBlock;
     //   block->buddy = NULL; Initialize buddy pointer
        set_footer(block);
        set_footer(newBlock);
    } else {
        block->free = 0;
    }
}

void* allocate_block(size_t size) {
    if (size == 0) return NULL;
    size_t total_size = HEADER_SIZE + size + FOOTER_SIZE;
    Block* block = find_free_block(total_size, currStrategy);
    if (block == NULL) {
        return NULL; // No suitable block found
    }
    split_block(block, total_size);
    return (void*)((char*)block + HEADER_SIZE);
}

// Helper: Find the previous block
Block* get_prev_block(Block* block) {
    Block* curr = heap_head;
    Block* prev = NULL;
    while (curr && curr != block) {
        prev = curr;
        curr = curr->next;
    }
    return prev;
}

void merge_block(Block* block) {
    if (block == NULL) return; // Check for NULL block

    // Merge with next block
    if (block->next != NULL && block->next->free &&
        ((char*)block + block->size == (char*)block->next)) {
        block->size += block->next->size;
        block->next = block->next->next;
        set_footer(block);
    }

    // Merge with previous block
    Block* prev = get_prev_block(block);
    if (prev != NULL && prev->free &&
        ((char*)prev + prev->size == (char*)block)) {
        prev->size += block->size;
        prev->next = block->next;
        set_footer(prev);
    }
}

void free_block(void* ptr) {
    if (ptr == NULL) return;
    Block* block = (Block*)((char*)ptr - HEADER_SIZE);
    if (block->free) {
        printf("Warning: Block at %p is already free, ignoring free request.\n", ptr);
        return;
    }

    block->free = 1;
    merge_block(block);
    for (int i = 0; i < allocation_count; ++i) {
        if (allocated_pointers[i] == ptr) {
            allocated_pointers[i] = NULL; // Mark as free in our tracking array
            break; 
        }
    }
}

void change_strategy(Strategy strategy) {
    currStrategy = strategy;
    printf("Allocation strategy changed to: %s\n", strategyNames[currStrategy]);
}

int main() {
    init_heap();
    int choice, size, free_index = 0;
    void *ptr_to_free;

    do {
        printf("\nMemory Allocator Menu (Strategy: %s)\n", strategyNames[currStrategy]);
        printf("1. Allocate Memory\n");
        printf("2. Free Memory\n");
        printf("3. Print Heap Status\n");
        printf("4. Change Allocation Strategy\n");
        printf("5. Quit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }

        switch (choice) {
            case 1:
                printf("Enter size to allocate: ");
                if (scanf("%d", &size) != 1 || size <= 0) {
                    printf("Invalid size. Please enter a positive integer.\n");
                    while (getchar() != '\n');
                    continue;
                }
                if (allocation_count < MAX_ALLOCATIONS) {
                    void* p = allocate_block(size);
                    if (p != NULL) {
                        printf("Allocated %d bytes at address: %p\n", size, p);
                        allocated_pointers[allocation_count++] = p; // Track allocation
                    } else {
                        printf("Memory allocation failed!\n");
                    }
                } else {
                    printf("Maximum allocations tracked. Free memory to allocate more.\n");
                }
                break;

            case 2:
                if (allocation_count > 0) {
                    printf("Enter index of allocation to free (0 to %d, -1 to free all): ", allocation_count - 1);
                    int free_choice;
                    if (scanf("%d", &free_choice) == 1) {
                        if (free_choice >= 0 && free_choice < allocation_count) {
                            if (allocated_pointers[free_choice] != NULL) {
                                free_block(allocated_pointers[free_choice]);
                                printf("Freed memory at %p\n", allocated_pointers[free_choice]);
                                allocated_pointers[free_choice] = NULL; // Clear the pointer
                                // Shift remaining pointers to fill the gap, but keep the count correct
                                // (Not strictly necessary for functionality, but keeps tracking cleaner)
                                //for (int i = free_choice; i < allocation_count - 1; ++i) {
                                //    allocated_pointers[i] = allocated_pointers[i + 1];
                                //}
                                //allocated_pointers[allocation_count - 1] = NULL; // Clear last element
                                //allocation_count--; // Decrement count only if we want to compact the array
                                int temp_count = 0;
                                for(int i = 0; i < allocation_count; ++i) { // Recount valid pointers
                                    if (allocated_pointers[i] != NULL) {
                                        allocated_pointers[temp_count++] = allocated_pointers[i];
                                    }
                                }
                                for (int i = temp_count; i < allocation_count; ++i) {
                                    allocated_pointers[i] = NULL;
                                }
                                allocation_count = temp_count;

                            } else {
                                printf("Allocation at index %d was already freed.\n", free_choice);
                            }
                        } else if (free_choice == -1) {
                            printf("Freeing all allocated blocks...\n");
                            for (int i = 0; i < allocation_count; ++i) {
                                if (allocated_pointers[i] != NULL) {
                                    free_block(allocated_pointers[i]);
                                    printf("Freed memory at %p\n", allocated_pointers[i]);
                                    allocated_pointers[i] = NULL;
                                }
                            }
                            allocation_count = 0;
                            printf("All blocks freed.\n");
                        }
                        else {
                            printf("Invalid index.\n");
                        }
                    } else {
                        printf("Invalid input.\n");
                        while (getchar() != '\n');
                    }
                } else {
                    printf("No memory blocks allocated to free.\n");
                }
                break;

            case 3:
                print_heap();
                break;

            case 4:
                printf("Change Allocation Strategy:\n");
                printf("1. First Fit\n");
                printf("2. Best Fit\n");
                printf("3. Worst Fit\n");
                printf("Enter strategy choice (1-3): ");
                int strategy_choice;
                if (scanf("%d", &strategy_choice) == 1) {
                    if (strategy_choice >= 1 && strategy_choice <= 3) {
                        change_strategy((Strategy)(strategy_choice - 1));
                    } else {
                        printf("Invalid strategy choice.\n");
                    }
                } else {
                    printf("Invalid input.\n");
                    while (getchar() != '\n');
                }
                break;

            case 5:
                printf("Quitting.\n");
                break;

            default:
                printf("Invalid choice. Please enter a number between 1 and 5.\n");
        }
    } while (choice != 5);

    return 0;
}
