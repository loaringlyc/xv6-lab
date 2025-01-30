# Lessons
1. prototypes of syscall functions in `syscall.c` has no arguments. 
   1. This is because the asm codes in `usys.pl` do nothing about arguments
   2. The argument should be dealt in the functions in `sysproc.c`
2. Memory layout of xv6
   <img src="https://p.ipic.vip/tntc6c.png" style="width: 40%;">
   <img src="https://p.ipic.vip/bi1vsg.png" style="width: 100%;">
   - User and kernel do not share the same VA space
   - kernel space
     - kernel stack starts at `0x80000000` address (0~80000000 has IO devices)
     - `entry.S`: Very first boot instructions
     - `trampoline`: Assembly code to switch between user and kernel
     - `kalloc.c`: Physical page allocator
     - `main.c`: Control initialization of other modules during boot
     - `syscall.c`: Dispatch system calls to handling function
     - `sysfile.c`: File-related system calls
     - `sysproc.c`: Process-related system calls
   - user space 
     - user code starts at `0x00000000` address. (e.g. `attack.asm` in this case)
3. starting xv6
   - When the RISC-V computer powers on, it initializes itself and runs a boot loader which is stored in read-only memory
   - The boot loader loads the xv6 kernel into memory at `0x80000000` 
   - Then, in machine mode, the CPU executes xv6 starting at `_entry`(kernel/entry.S:7). It setup a stack so that xv6 can run C code
     - `sp` was set to `stack0 + 4096` because it's top of stack, and the stack grows down
   - Then goes to `start.c`. 
   - Then goes to `main.c`. 
     - initializes several devices and subsystems
     - creates the first process by calling `userinit()`
     - execute `init.c` in user code, which creates a console
4. attack xv6
   - `fork()`: use `uvmcopy()` to copy memory(both page table and user memory) to child
   - `exec()`: get page table first by `proc_pagetable()`; then 
