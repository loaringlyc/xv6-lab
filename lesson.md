# Lesson
### Chapter 3
1. paging hardware
   <img src="https://p.ipic.vip/l0hos4.png" style="width: 60%;">
   - potential downside: must load three PTEs from memory => use TLB
   - every CPU has a root page table, so every CPU has a `satp` register
2. kernel address space
   <img src="https://p.ipic.vip/bi1vsg.png" style="width: 100%;">
   - QEMU simulates a computer that includes RAM (physical memory) starting at physical address `0x80000000` and continuing through at least `0x88000000`
   -  QEMU exposes the device interfaces to software as memory-mapped control registers that sit below `0x80000000` in the physical address space.
   -  direct mapping: mapping the resources at virtual addresses that are equal to the physical address.
      -  most direct 
      -  not direct mapping: 
         -  The trampoline page: both mapped by kernel&user address space
         -  The kernel stack pages: below it xv6 can leave an unmapped guard page (deal with overflow)
3. generating address space `vm.c`
   - central data structure: `pagetable_t`: a pointer to a RISC-V root page-table page
   - central functions: 
     - `walk`: finds the PTE for a virtual address
     - `mappages`: installs PTEs for new mappings
     - `copyout` and `copyin`: copy data to and from user virtual addresses
   - other functions:
     - `kvminithart`: install the kernel page table (`sfence.vma` instruction in xv6 to flush TLB -> first flush ensures preceding updates to the page table have completed using old TLB)
4. physical memory allocation `kalloc.c`
   - The allocator’s data structure: a free list of physical memory pages that are available for allocation
   - Every free page's element: a struct `run` (a singly-linked list node)
   - Initialize the allocator: `main` calls `kinit`
     - In `kinit`, every page between the end of the kernel and PHYSTOP is held by freelist
     - `freerange`: let pages between init to end belong to freelist
5. Process address space
   <img src="https://p.ipic.vip/rhygs6.png" style="width: 100%;">
   - Each process has its own page table
   - User memory: starts at virtual address 0, grow up to MAXVA (principle 256G)
4. Codes
   - `sbrk`
   - `exec`
     - opens the named binary path using `namei`
     - reads the ELF header (the format is in `elf.h`)
     - `loadseg`: Load a program segment into pagetable at virtual address va
     - Note: the addresses in the ELF file may refer to the kernel accidentally or on purpose

### Programming
1. Each page table contains 512 ptes
2. How to decide whether a page directory is the final level: not read/write/executable
3. How to decide virtual address of a page? In programming it
