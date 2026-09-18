/* =============================================================================
 * SENG21213-OS :: Main Kernel  (Stage 0 – Foundations)
 * File   : kernel/kernel.c
 *
 * PURPOSE
 *   This is the heart of your operating system. Right now it:
 *     1. Initialises VGA text-mode display
 *     2. Initialises the keyboard driver
 *     3. Prints a splash screen
 *     4. Runs a minimal interactive shell ("ksh")
 *
 * ASSIGNMENT MILESTONES  (what YOU will add in later lectures)
 *   Lecture  9  – Process Management  →  process.h / process.c / scheduler.c
 *   Lecture 10  – Threads             →  thread.h  / thread.c
 *   Lecture 11  – Memory Management   →  pmm.h     / pmm.c / vmm.c
 *   Lecture 12  – File System         →  fs.h      / fs.c
 *
 * CODING CONVENTION
 *   - Prefix kernel-internal functions with k_ (e.g. k_strcmp)
 *   - All driver APIs live in their own .h/.c pair
 *   - NEVER call malloc – use the PMM you build in Lecture 11
 * ============================================================================*/

#include "vga.h"
#include "keyboard.h"
#include "../include/types.h"
#include "process.h"
#include "scheduler.h"
#include "thread.h"
#include "mutex.h" 
#include "semaphore.h"
#include "pmm.h"
#include "vmm.h"
#include "fs.h"
#include "ramdisk.h"

/* ---------------------------------------------------------------------------
 * Forward declarations of shell commands
 * --------------------------------------------------------------------------*/
static void cmd_help(void);
static void cmd_clear(void);
static void cmd_about(void);
static void cmd_echo(const char *args);
static void cmd_mem(void);

/* ---------------------------------------------------------------------------
 * Utility: minimal string helpers (no libc in a freestanding kernel!)
 * --------------------------------------------------------------------------*/
static int k_strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) { a++; b++; }
    return (uint8_t)*a - (uint8_t)*b;
}

static int k_strncmp(const char *a, const char *b, size_t n) {
    while (n-- && *a && (*a == *b)) { a++; b++; }
    return n == (size_t)-1 ? 0 : (uint8_t)*a - (uint8_t)*b;
}

static size_t k_strlen(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

/* Skip leading spaces */
static const char *k_ltrim(const char *s) {
    while (*s == ' ') s++;
    return s;
}

/* ---------------------------------------------------------------------------
 * Splash Screen
 * --------------------------------------------------------------------------*/
static void print_splash(void) {
    vga_clear(VGA_BLACK);

    /* Top banner box */
    vga_draw_box(0, 0, 7, 80, VGA_LIGHT_MAGENTA);

    vga_set_cursor(1, 2);
    vga_puts_color("  SENG21213-OS  |  Computer Architecture & Operating Systems",
                   VGA_YELLOW, VGA_BLACK);

    vga_set_cursor(2, 2);
    vga_puts_color("  Stage 0: Kernel Foundations", VGA_LIGHT_CYAN, VGA_BLACK);

    vga_set_cursor(3, 2);
    vga_puts_color("  Faculty of Engineering – Department of Software Engineering",
                   VGA_LIGHT_GREY, VGA_BLACK);

    vga_set_cursor(4, 2);
    vga_puts_color("  Built by students, for students.  Type 'help' to begin.",
                   VGA_LIGHT_GREEN, VGA_BLACK);

    vga_set_cursor(5, 2);
    vga_puts_color("  CPU: i686 (32-bit Protected Mode)  |  Display: VGA 80x25",
                   VGA_DARK_GREY, VGA_BLACK);

    vga_set_cursor(8, 0);
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("  Welcome! This kernel was compiled from source and booted entirely\n");
    vga_puts("  from bare metal. There is no Linux or Windows underneath – only\n");
    vga_puts("  the code you and your team write.\n");
    vga_puts("\n");
    vga_puts("  Assignment milestones to implement:\n");
    vga_puts_color("    [L09] ", VGA_YELLOW, VGA_BLACK);
    vga_puts("Process Management  – PCB, ready queue, round-robin scheduler\n");
    vga_puts_color("    [L10] ", VGA_YELLOW, VGA_BLACK);
    vga_puts("Threads & Sync      – kernel threads, mutex, semaphore\n");
    vga_puts_color("    [L11] ", VGA_YELLOW, VGA_BLACK);
    vga_puts("Memory Management   – physical page allocator, virtual memory\n");
    vga_puts_color("    [L12] ", VGA_YELLOW, VGA_BLACK);
    vga_puts("File System         – RAM disk, FAT-like directory structure\n");
    vga_puts("\n");
}

/* ---------------------------------------------------------------------------
 * Shell command implementations
 * --------------------------------------------------------------------------*/
static void cmd_help(void) {
    vga_puts_color("\n  SENG21213-OS Shell Commands\n", VGA_YELLOW, VGA_BLACK);
    vga_puts("  ─────────────────────────────────────────────\n");
    vga_puts("  help    – Show this help message\n");
    vga_puts("  clear   – Clear the screen\n");
    vga_puts("  about   – About this OS and course\n");
    vga_puts("  echo    – Echo text to screen\n");
    vga_puts("  mem     – Memory map (stub)\n");
    vga_puts_color("\n  Milestones (to implement):\n", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("  ps      – [L09] List processes\n");
    vga_puts("  kill    – [L09] Terminate a process\n");
    vga_puts("  threads – [L10] List kernel threads\n");
    vga_puts("  free    – [L11] Show free memory\n");
    vga_puts("  ls      – [L12] List files\n");
    vga_puts("  cat     – [L12] Print file contents\n\n");
}

static void cmd_clear(void) {
    vga_clear(VGA_BLACK);
}

static void cmd_about(void) {
    vga_puts_color("\n  About SENG21213-OS\n", VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("  ─────────────────────────────────────────────\n");
    vga_puts("  Architecture : x86 (i686), 32-bit Protected Mode\n");
    vga_puts("  Bootloader   : Custom MBR (NASM)\n");
    vga_puts("  Kernel       : Freestanding C (GCC, no libc)\n");
    vga_puts("  VM Target    : QEMU (qemu-system-i386)\n");
    vga_puts("  Course       : SENG 21213 – Sem 2\n");
    vga_puts("  Reference    : Stallings, OS: Internals & Design Principles\n\n");
}

static void cmd_echo(const char *args) {
    vga_puts("  ");
    vga_puts(args);
    vga_puts("\n");
}

static void cmd_mem(void)
{
    vga_puts("\n  Physical Memory Manager\n");
    vga_puts("  -----------------------\n");

    vga_printf("  Used pages : %d\n", pmm_get_used_pages());
    vga_printf("  Free pages : %d\n", pmm_get_free_pages());
    vga_printf("  Page size  : %d bytes\n", PAGE_SIZE);

    vga_puts("\n");
}

static void cmd_memtest(void)
{
    void *page1;
    void *page2;

    vga_puts("\n  PMM Allocation Test\n");
    vga_puts("  -------------------\n");

    vga_printf("  Free before : %d\n", pmm_get_free_pages());

    page1 = pmm_alloc_page();
    page2 = pmm_alloc_page();

    vga_printf("  Page 1 addr : 0x%x\n", (uint32_t)page1);
    vga_printf("  Page 2 addr : 0x%x\n", (uint32_t)page2);

    vga_printf("  Free after allocation : %d\n",
               pmm_get_free_pages());

    pmm_free_page(page1);
    pmm_free_page(page2);

    vga_printf("  Free after free : %d\n",
               pmm_get_free_pages());

    vga_puts("\n");
}

static void cmd_vmmtest(void)
{
    uint32_t physical;
    uint32_t mapped;

    vga_puts("\n  VMM Mapping Test\n");
    vga_puts("  ----------------\n");

    physical = (uint32_t)pmm_alloc_page();

    vmm_map_page(0x00200000, physical);
    mapped = vmm_get_mapping(0x00200000);

    vga_printf("  Physical page : 0x%x\n", physical);
    vga_printf("  Virtual addr  : 0x%x\n", 0x00200000);
    vga_printf("  Mapping       : 0x%x\n", mapped);

    if (mapped == physical) {
        vga_puts("  Result        : PASS\n");
    } else {
        vga_puts("  Result        : FAIL\n");
    }

    pmm_free_page((void *)physical);

    vga_puts("\n");
}

static void cmd_fstest(void)
{
    char buffer[RAMDISK_BLOCK_SIZE];

    vga_puts("\n  File System Test\n");
    vga_puts("  ----------------\n");

    if (fs_create("hello.txt") == 0) {
        vga_puts("  Create file : PASS\n");
    } else {
        vga_puts("  Create file : FAIL\n");
    }

    if (fs_write("hello.txt", "Hello from SENG21213 OS!") == 0) {
        vga_puts("  Write file  : PASS\n");
    } else {
        vga_puts("  Write file  : FAIL\n");
    }

    if (fs_read("hello.txt", buffer) == 0) {
        vga_puts("  Read file   : PASS\n");
        vga_puts("  Content     : ");
        vga_puts(buffer);
        vga_puts("\n");
    } else {
        vga_puts("  Read file   : FAIL\n");
    }

    fs_list();

    vga_puts("\n");
}

static void cmd_ls(void)
{
    fs_list();
}

static void cmd_cat(const char *filename)
{
    char buffer[RAMDISK_BLOCK_SIZE];

    if (filename == NULL || k_strlen(filename) == 0) {
        vga_puts("  Usage: cat <filename>\n");
        return;
    }

    if (fs_read(filename, buffer) == 0) {
        vga_puts("  ");
        vga_puts(buffer);
        vga_puts("\n");
    } else {
        vga_puts("  File not found: ");
        vga_puts(filename);
        vga_puts("\n");
    }
}

static void cmd_rm(const char *filename)
{
    if (filename == NULL || k_strlen(filename) == 0) {
        vga_puts("  Usage: rm <filename>\n");
        return;
    }

    if (fs_delete(filename) == 0) {
        vga_puts("  File deleted: ");
        vga_puts(filename);
        vga_puts("\n");
    } else {
        vga_puts("  File not found: ");
        vga_puts(filename);
        vga_puts("\n");
    }
}

/* ---------------------------------------------------------------------------
 * Shell process
 * --------------------------------------------------------------------------*/
static char  shell_buf[256];
static char  prompt[] = "\n  ksh> ";

static void shell_run(void) {
    vga_puts_color("\n  Kernel Shell ready. Type 'help' for commands.\n",
                   VGA_LIGHT_GREEN, VGA_BLACK);

    while (true) {
        vga_puts_color(prompt, VGA_LIGHT_GREEN, VGA_BLACK);
        kb_readline(shell_buf, sizeof(shell_buf));

        /* Trim leading whitespace */
        const char *cmd = k_ltrim(shell_buf);
        if (k_strlen(cmd) == 0) continue;

        /* Dispatch */
        if (k_strcmp(cmd, "help")  == 0) { cmd_help();  continue; }
        if (k_strcmp(cmd, "clear") == 0) { cmd_clear(); continue; }
        if (k_strcmp(cmd, "about") == 0) { cmd_about(); continue; }
        if (k_strcmp(cmd, "mem")   == 0) {
           cmd_mem();
           continue;
        }

        if (k_strcmp(cmd, "memtest") == 0) {
        cmd_memtest();
        continue;
        }
        
        if (k_strcmp(cmd, "vmmtest") == 0) {
        cmd_vmmtest();
        continue;
        }

        if (k_strcmp(cmd, "fstest") == 0) {
        cmd_fstest();
        continue;
        }

        if (k_strcmp(cmd, "ls") == 0) {
        cmd_ls();
        continue;
        }

        if (k_strncmp(cmd, "cat ", 4) == 0) {
        cmd_cat(k_ltrim(cmd + 4));
        continue;
        }

        if (k_strcmp(cmd, "cat") == 0) {
        cmd_cat("");
        continue;
        }

        if (k_strncmp(cmd, "rm ", 3) == 0) {
        cmd_rm(k_ltrim(cmd + 3));
        continue;
        }

        if (k_strcmp(cmd, "rm") == 0) {
        cmd_rm("");
        continue;
        }

        if (k_strcmp(cmd, "ps") == 0) {
            process_list();
            continue;
        }
 
        if (k_strcmp(cmd, "threads") == 0) {
        thread_list();
        continue;
        }        
          
        if (k_strcmp(cmd, "run") == 0) {
        scheduler_start();
        continue;
        }

        if (k_strcmp(cmd, "runthreads") == 0) {
        thread_start();
        continue;
        }

        if (k_strncmp(cmd, "echo ", 5) == 0) {
            cmd_echo(k_ltrim(cmd + 5));
            continue;
        }

        /* Milestone stubs */
        if (k_strcmp(cmd, "kill") == 0 ||
            k_strcmp(cmd, "free") == 0) {
            vga_puts_color("  [TODO] This command is not yet implemented.\n",
                           VGA_YELLOW, VGA_BLACK);
            vga_puts("  Implement it as part of your lecture assignment.\n");
            continue;
        }

        vga_puts_color("  Unknown command: ", VGA_LIGHT_RED, VGA_BLACK);
        vga_puts(cmd);
        vga_puts("\n  Type 'help' for a list of commands.\n");
    }
}

/* ---------------------------------------------------------------------------
 * Kernel entry point – called from kernel_entry.asm
 * --------------------------------------------------------------------------*/
static void test_process_1(void)
{
    while (true) {
        vga_puts("Process 1 running\n");
        process_yield();
    }
}

static void test_process_2(void)
{
    while (true) {
        vga_puts("Process 2 running\n");
        process_yield();
    }
}

static mutex_t test_mutex;
static semaphore_t test_semaphore;

static void test_thread_1(void)
{
    while (true) {
        semaphore_wait(&test_semaphore);

        vga_puts("Thread 1 acquired semaphore\n");

        semaphore_signal(&test_semaphore);

        thread_yield();
    }
}

static void test_thread_2(void)
{
    while (true) {
        semaphore_wait(&test_semaphore);

        vga_puts("Thread 2 acquired semaphore\n");

        semaphore_signal(&test_semaphore);

        thread_yield();
    }
}

void kernel_main(void) {
    vga_init();
    kb_init();

    process_init();
    scheduler_init();
        
    process_create(test_process_1);
    process_create(test_process_2); 

    thread_init();
    mutex_init(&test_mutex);
    semaphore_init(&test_semaphore, 1);
    
    pmm_init();
    vmm_init(); 
    ramdisk_init();
    fs_init(); 

    thread_create(test_thread_1);
    thread_create(test_thread_2);

    print_splash();
    shell_run();

    /* Should never reach here */
    __asm__ __volatile__("hlt");
}

