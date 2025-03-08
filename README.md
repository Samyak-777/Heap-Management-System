# `Heap Management System`

This project implements a simple **heap memory manager** in C with custom **allocate** and **free** functions. It maintains a linked list of memory blocks, supports block **splitting and merging**, and minimizes fragmentation.

## Features 

- **Custom Memory Allocation** – Implements `allocate(size)` to allocate memory from a static heap.  
- **Dynamic Freeing & Merging** – `free(ptr)` deallocates memory and merges adjacent free blocks.  
- **Block Splitting** – Allocates only the required size, leaving the rest as a free block.  
- **Allocation Strategies** – Supports **First Fit, Best Fit, and Worst Fit** selection.  
- **Heap Debugging** – Includes `print_heap()` to visualize memory allocation.  

## How It Works

### 🔹 Initialization :
- `init_heap()`: Creates a single large free block covering the heap.

### 🔹 Memory Allocation :
- `allocate(size)`:  
  - Finds a suitable free block (using the selected allocation strategy).  
  - Splits if necessary.  
  - Returns a pointer to the allocated memory.

### 🔹 Memory Freeing & Merging :
- `free(ptr)`:  
  - Marks the block as free.  
  - Merges adjacent free blocks for efficient reuse.

## Compilation & Execution :
 Use your preferred C compiler. For example, with GCC:  
```bash
gcc -o heap_management heap_management.c
./heap_management
```

