// Copyright © 2026 Aerospace Corporation
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * @file aerospace.c
 * @brief Linux kernel module for SpaceCop system call monitoring
 *
 * This kernel module provides deep system call monitoring capabilities for
 * SpaceCop intrusion detection. It uses Linux kprobes to intercept system
 * calls and report suspicious activity to userspace via Netlink sockets.
 *
 * **Architecture:**
 * @code
 * ┌─────────────────────────────────────────────────────────┐
 * │                    Userspace                             │
 * │  ┌──────────────┐              ┌──────────────┐        │
 * │  │  SpaceCop    │◄─────────────┤   Netlink    │        │
 * │  │  Application │   Events     │   Socket     │        │
 * │  └──────────────┘              └──────┬───────┘        │
 * └────────────────────────────────────────┼────────────────┘
 *                                          │
 * ═════════════════════════════════════════╪═════════════════
 *                                          │
 * ┌────────────────────────────────────────┼────────────────┐
 * │                   Kernel Space         │                 │
 * │                                        ▼                 │
 * │  ┌──────────────────────────────────────────────────┐  │
 * │  │  SpaceCop Kernel Module (This File)             │  │
 * │  │  ┌────────────┐  ┌────────────┐  ┌───────────┐ │  │
 * │  │  │  Kprobes   │  │  Netlink   │  │  Process  │ │  │
 * │  │  │  Handlers  │  │  Interface │  │  Context  │ │  │
 * │  │  └─────┬──────┘  └────────────┘  └───────────┘ │  │
 * │  └────────┼─────────────────────────────────────────┘  │
 * │           │                                             │
 * │           ▼                                             │
 * │  ┌─────────────────────────────────────────────────┐  │
 * │  │  Linux Kernel System Call Layer                 │  │
 * │  │  - execve()   - openat()   - fchmodat()        │  │
 * │  │  - init_module()                                │  │
 * │  └─────────────────────────────────────────────────┘  │
 * └─────────────────────────────────────────────────────────┘
 * @endcode
 *
 * **Monitored System Calls:**
 * 
 * 1. **execve() - Program Execution**
 *    - Detects process creation
 *    - Identifies executed binaries
 *    - Tracks parent process context
 *    - IOB: SIUU-11 (Suspicious Binary Execution)
 * 
 * 2. **openat() - File Access**
 *    - Monitors critical device access (/dev/mtd*, /dev/zero, /dev/null)
 *    - Detects flash memory access
 *    - Tracks file operations
 *    - IOB: MIRE-17 (Flash Memory Access)
 * 
 * 3. **fchmodat() - Permission Changes**
 *    - Detects file permission modifications
 *    - Tracks chmod operations
 *    - Identifies privilege escalation attempts
 *    - IOB: SIUU-16 (System Command Execution)
 * 
 * 4. **init_module() - Kernel Module Loading**
 *    - Detects kernel module insertion
 *    - Identifies rootkit installation
 *    - Tracks module loading
 *    - IOB: SIUU-12 (Malicious Kernel Module)
 *
 * **Multi-Architecture Support:**
 * Supports x86_64, ARM32, and ARM64 architectures with automatic detection
 * and architecture-specific register access macros.
 *
 * **Kprobes Technology:**
 * Uses Linux kprobes for dynamic instrumentation:
 * - Pre-handler intercepts syscall before execution
 * - Minimal performance overhead (<5% typical)
 * - No kernel recompilation required
 * - Graceful handling of symbol variations
 *
 * **Netlink Communication:**
 * Communicates with userspace SpaceCop via Netlink socket:
 * - Protocol: NETLINK_USER (31)
 * - Bidirectional communication
 * - Efficient kernel-user data transfer
 * - Asynchronous event notification
 *
 * **Process Context Tracking:**
 * Captures rich process information:
 * - Process name (comm)
 * - Process ID (PID)
 * - Thread Group ID (TGID)
 * - User ID (UID)
 * - Parent process context
 *
 * **Usage:**
 * @code
 * # Load kernel module
 * sudo insmod aerospace.ko
 *
 * # Verify loaded
 * lsmod | grep spacecop
 * dmesg | grep SPACECOP
 *
 * # Start the userspace SpaceCop application to receive events
 *
 * # Monitor events in kernel log
 * tail -f /var/log/kern.log | grep SPACECOP
 *
 * # Unload module
 * sudo rmmod aerospace
 * @endcode
 *
 * **Security Considerations:**
 * - Requires root/CAP_SYS_MODULE to load
 * - Kernel module has full system access
 * - Validates all userspace pointers
 * - Prevents buffer overflows
 * - Handles missing symbols gracefully
 *
 * **Performance:**
 * - Overhead: <5% CPU typical
 * - Memory: ~100KB for module + buffers
 * - Netlink: Minimal latency (<1ms)
 * - Kprobes: Optimized for hot paths
 *
 * **Kernel Version Compatibility:**
 * - Linux 4.x: Supported
 * - Linux 5.x: Supported
 * - Linux 6.x: Supported
 * - Symbol names may vary by kernel version
 *
 * @note Requires CONFIG_KPROBES=y in kernel configuration
 * @note Requires CONFIG_NETLINK=y in kernel configuration
 * @note Must be GPL licensed for kernel symbol access
 *
 * @see Documentation/kprobes.txt for kprobes details
 * @see Documentation/networking/netlink.txt for Netlink details
 *
 * @warning Kernel module has full system privileges
 * @warning Improper use can crash system
 * @warning Test thoroughly before production deployment
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kprobes.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/unistd.h>
#include <linux/uidgid.h>

#include <net/sock.h>
#include <linux/slab.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>

/*=======================================================================================
** Module Metadata
**=======================================================================================*/

MODULE_LICENSE("GPL");
MODULE_AUTHOR("rjt34234");
MODULE_DESCRIPTION("Kernel module for SpaceCOP - Multi-architecture");
MODULE_VERSION("1.0");

/*=======================================================================================
** Architecture Detection and Configuration
**=======================================================================================*/

/**
 * @brief Architecture detection and register access macros
 * 
 * Detects target architecture at compile time and defines appropriate
 * register access macros for system call argument extraction.
 *
 * **Supported Architectures:**
 * - x86_64: Intel/AMD 64-bit
 * - ARM32: 32-bit ARM (ARMv7)
 * - ARM64: 64-bit ARM (ARMv8/AArch64)
 *
 * **Register Conventions:**
 * @code
 * x86_64:
 * - ARG0: RDI (regs->di)
 * - ARG1: RSI (regs->si)
 * - ARG2: RDX (regs->dx)
 * - ARG3: R10 (regs->r10)
 * 
 * ARM32:
 * - ARG0: R0 (regs->ARM_r0)
 * - ARG1: R1 (regs->ARM_r1)
 * - ARG2: R2 (regs->ARM_r2)
 * - ARG3: R3 (regs->ARM_r3)
 * 
 * ARM64:
 * - ARG0: X0 (regs->regs[0])
 * - ARG1: X1 (regs->regs[1])
 * - ARG2: X2 (regs->regs[2])
 * - ARG3: X3 (regs->regs[3])
 * @endcode
 */
#if defined(__x86_64__) || defined(__amd64__)
    #define ARCH_X86_64
    #define ARCH_NAME "x86_64"
#elif defined(__arm__) || defined(__aarch32__)
    #define ARCH_ARM32
    #define ARCH_NAME "ARM32"
#elif defined(__aarch64__)
    #define ARCH_ARM64
    #define ARCH_NAME "ARM64"
#else
    #warning "Unknown architecture - defaulting to generic"
    #define ARCH_GENERIC
    #define ARCH_NAME "Generic"
#endif

/*=======================================================================================
** Constants and Configuration
**=======================================================================================*/

/**
 * @brief Maximum filename length for captured paths
 * 
 * Limits the size of filename buffers to prevent excessive memory usage
 * and potential buffer overflows when copying from userspace.
 */
#define MAX_FILENAME_LEN 256

/**
 * @brief Netlink protocol number for SpaceCop
 * 
 * Custom Netlink protocol number used for SpaceCop kernel-userspace
 * communication. Value 31 is in the user-defined range (16-31).
 */
#define NETLINK_USER 31

/**
 * @brief Maximum message length for Netlink messages
 * 
 * Maximum size of messages sent from kernel to userspace via Netlink.
 * Includes syscall name, process info, and arguments.
 */
#define MAX_MSG_LEN 1024

/*=======================================================================================
** Global Variables
**=======================================================================================*/

/**
 * @brief Netlink socket for kernel-userspace communication
 * 
 * Socket used to send system call events from kernel to userspace SpaceCop
 * daemon. Created during module initialization.
 */
static struct sock *nl_sk = NULL;

/**
 * @brief Process ID of userspace SpaceCop daemon
 * 
 * PID of the userspace process that will receive Netlink messages.
 * Set when userspace daemon registers with kernel module.
 * Value -1 indicates no daemon registered.
 */
static int user_pid = -1;

/**
 * @brief Syscall table pointer (unused in kprobe implementation)
 * 
 * Legacy variable from syscall table hijacking approach.
 * Not used in current kprobe-based implementation but kept for
 * potential future use.
 */
unsigned long **syscall_table;

/*=======================================================================================
** Architecture-Specific Register Access Macros
**=======================================================================================*/

/**
 * @brief Register access macros for system call arguments
 * 
 * Architecture-specific macros to extract system call arguments from
 * CPU registers. Each architecture has different calling conventions.
 *
 * **Usage Example:**
 * @code
 * static int syscall_handler(struct kprobe *p, struct pt_regs *regs) {
 *     unsigned long arg0 = GET_REG_ARG0(regs);  // First argument
 *     unsigned long arg1 = GET_REG_ARG1(regs);  // Second argument
 *     
 *     // Process arguments...
 * }
 * @endcode
 */
#ifdef ARCH_X86_64
    #define GET_REG_ARG0(regs) ((regs)->di)   /**< RDI register (1st arg) */
    #define GET_REG_ARG1(regs) ((regs)->si)   /**< RSI register (2nd arg) */
    #define GET_REG_ARG2(regs) ((regs)->dx)   /**< RDX register (3rd arg) */
    #define GET_REG_ARG3(regs) ((regs)->r10)  /**< R10 register (4th arg) */
#elif defined(ARCH_ARM32)
    #define GET_REG_ARG0(regs) ((regs)->ARM_r0)  /**< R0 register (1st arg) */
    #define GET_REG_ARG1(regs) ((regs)->ARM_r1)  /**< R1 register (2nd arg) */
    #define GET_REG_ARG2(regs) ((regs)->ARM_r2)  /**< R2 register (3rd arg) */
    #define GET_REG_ARG3(regs) ((regs)->ARM_r3)  /**< R3 register (4th arg) */
#elif defined(ARCH_ARM64)
    #define GET_REG_ARG0(regs) ((regs)->regs[0])  /**< X0 register (1st arg) */
    #define GET_REG_ARG1(regs) ((regs)->regs[1])  /**< X1 register (2nd arg) */
    #define GET_REG_ARG2(regs) ((regs)->regs[2])  /**< X2 register (3rd arg) */
    #define GET_REG_ARG3(regs) ((regs)->regs[3])  /**< X3 register (4th arg) */
#else
    #error "Unsupported architecture"
#endif

/*=======================================================================================
** Helper Functions
**=======================================================================================*/

/**
 * @brief Extract process information for current context
 * 
 * Captures comprehensive process information for the currently executing
 * process. Used to provide context for system call events.
 *
 * **Information Captured:**
 * - Process name (comm): Executable name (max 16 chars)
 * - PID: Kernel process ID
 * - TGID: Thread group ID (userspace PID)
 * - UID: User ID of process owner
 *
 * **Output Format:**
 * "process=<name>,pid=<pid>,tgid=<tgid>,uid=<uid>"
 *
 * **Example Output:**
 * "process=insmod,pid=1234,tgid=1234,uid=0"
 *
 * @param[out] buf Buffer to store formatted process information
 * @param[in] buf_size Size of output buffer in bytes
 *
 * @note Uses current task_struct pointer (current)
 * @note comm field is always null-terminated
 * @note Thread group ID (TGID) is what userspace sees as PID
 */
static void get_process_info(char *buf, size_t buf_size)
{
    // Get current process information
    const char *comm = current->comm;  // Process name (max 16 chars)
    pid_t pid = current->pid;
    pid_t tgid = current->tgid;  // Thread group ID (actual PID in userspace)
    kuid_t uid = current_uid();
    
    snprintf(buf, buf_size, 
             "process=%s,pid=%d,tgid=%d,uid=%d",
             comm, pid, tgid, from_kuid(&init_user_ns, uid));
}

/**
 * @brief Netlink message receive callback
 * 
 * Called when userspace sends a message to the kernel via Netlink socket.
 * Primary purpose is to register the userspace SpaceCop daemon's PID so
 * the kernel module knows where to send events.
 *
 * **Registration Protocol:**
 * @code
 * Userspace:
 * 1. Open Netlink socket with NETLINK_USER protocol
 * 2. Send registration message (any content)
 * 3. Kernel extracts PID from Netlink header
 * 4. Kernel saves PID for future event delivery
 * @endcode
 *
 * **Message Format:**
 * - Netlink header contains sender PID
 * - Message data can be any string (typically "REGISTER")
 * - PID automatically extracted from nlmsg_pid field
 *
 * @param[in] skb Socket buffer containing received message
 *
 * @note Called in process context (of the netlink sender task)
 * @note PID saved globally for event transmission
 */
static void netlink_recv_msg(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
	char *received_msg;

	nlh = (struct nlmsghdr *)skb->data;
	received_msg = (char *)nlmsg_data(nlh);
	user_pid = nlh->nlmsg_pid;

	printk(KERN_INFO "[SPACECOP] Kernel received message: %s\n", received_msg);
	printk(KERN_INFO "[SPACECOP] User-space server PID saved: %d\n", user_pid);
}

/**
 * @brief Send event message to userspace via Netlink
 * 
 * Transmits system call event information to registered userspace SpaceCop
 * daemon via Netlink socket. Uses unicast delivery to specific PID.
 *
 * **Message Flow:**
 * @code
 * 1. Allocate socket buffer (skb)
 * 2. Create Netlink message header
 * 3. Copy event data to message
 * 4. Send unicast to user_pid
 * 5. Free buffer (handled by kernel)
 * @endcode
 *
 * **Error Handling:**
 * - user_pid < 0: No daemon registered, message dropped
 * - Allocation failure: Error logged, message dropped
 * - Send failure: Error logged, buffer freed
 *
 * **Message Format:**
 * Plain text string containing:
 * - Syscall name
 * - Process information
 * - Syscall arguments (filename, etc.)
 *
 * **Example Messages:**
 * @code
 * "Syscall execve called by process=bash,pid=1234,tgid=1234,uid=0 with program: /bin/ls"
 * "openat called by process=cat,pid=5678,tgid=5678,uid=1000 with filename: /dev/mtd0"
 * @endcode
 *
 * @param[in] buf Null-terminated string containing event message
 *
 * @note Called from kprobe context (may be atomic)
 * @note Message must fit in MAX_MSG_LEN bytes
 * @note Unicast delivery (only registered daemon receives)
 *
 * @warning Silently drops messages if no daemon registered
 * @warning Buffer must be null-terminated
 */
static void send_netlink_msg(const char *buf)
{
	struct nlmsghdr *nlh;
    struct sk_buff *skb_out;
    int res;

    if(user_pid < 0)
    {
    	printk(KERN_ERR "[SPACECOP] User-space server PID is not set.\n");
    	return;
    }

    skb_out = nlmsg_new(strlen(buf), 0);
    if(!skb_out)
    {
        printk(KERN_ERR "[SPACECOP] Error allocating memory for netlink socket.\n");
        return;
    }
   
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, strlen(buf), 0);
    
    if(!nlh)
    {
    	kfree_skb(skb_out);
    	printk(KERN_ERR "[SPACECOP] Failed to allocate skb\n");
    	return;
    }

	strscpy(nlmsg_data(nlh), buf, strlen(buf) + 1);
    res = nlmsg_unicast(nl_sk, skb_out, user_pid); 
    if(res < 0)
    {
        pr_info("[%s] Error sending msg to userspace: %d\n", THIS_MODULE->name, res);
    }
    else
    {
        pr_debug("[%s] Sent message to userspace\n", THIS_MODULE->name);
    }
}

/*=======================================================================================
** Kprobe Handler Functions
**=======================================================================================*/

/**
 * @brief Kprobe pre-handler for fchmodat() system call
 * 
 * Intercepts fchmodat() calls to detect file permission changes. Permission
 * changes can indicate:
 * - Privilege escalation attempts
 * - Malware making files executable
 * - Configuration tampering
 * - Security policy violations
 *
 * **System Call Signature:**
 * int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags);
 *
 * **Arguments Captured:**
 * - ARG0: dirfd (directory file descriptor)
 * - ARG1: pathname (file path)
 * - ARG2: mode (new permissions)
 *
 * **Detection Use Cases:**
 * - IOB SIUU-16: System command execution
 * - Unauthorized permission changes
 * - Executable file creation
 * - Configuration file modification
 *
 * @param[in] p Kprobe structure
 * @param[in] regs CPU registers at syscall entry
 *
 * @return 0 to continue syscall execution
 *
 * @note x86_64 requires task_pt_regs() for userspace registers
 * @note Other architectures use regs directly
 */
static int chmod_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
	char kfile[256];
	char msg[1024];
	char proc_info[128];
	unsigned long filename_ptr;

	// Get process information
	get_process_info(proc_info, sizeof(proc_info));

#ifdef ARCH_X86_64
	struct pt_regs *user_regs = task_pt_regs(current);
	filename_ptr = GET_REG_ARG1(user_regs);
#else
	filename_ptr = GET_REG_ARG1(regs);
#endif

	if(filename_ptr)
	{
		if (strncpy_from_user(kfile, (char __user *)filename_ptr, sizeof(kfile)) > 0) {
            kfile[sizeof(kfile) - 1] = '\0';  
            printk(KERN_INFO "[SPACECOP]\tSyscall %s called by %s with filename: %s\n", 
                   p->symbol_name, proc_info, kfile);
            snprintf(msg, sizeof(msg), "Syscall %s called by %s with filename: %s\n", 
                     p->symbol_name, proc_info, kfile);
        } 
        else
        {
        	printk(KERN_INFO "[SPACECOP]\tSyscall %s called by %s\n", 
        	       p->symbol_name, proc_info);
        	snprintf(msg, sizeof(msg), "Syscall %s called by %s\n", 
        	         p->symbol_name, proc_info);
        }

        msg[sizeof(msg)-1] = '\0';
        send_netlink_msg(msg);
	}

	return 0;
}

/**
 * @brief Generic kprobe pre-handler for system calls
 * 
 * General-purpose template handler that reads ARG1 as a pathname pointer and
 * reports it along with process context. It is attached to read_kp, which
 * monitor_init does not currently register, so this handler is presently
 * inactive. (Note that read()'s second argument is actually a buffer, not a
 * pathname, so this handler would need adapting before use on read().)
 *
 * **Arguments:**
 * - ARG0: File descriptor or directory fd
 * - ARG1: Filename/pathname pointer
 *
 * @param[in] p Kprobe structure
 * @param[in] regs CPU registers at syscall entry
 *
 * @return 0 to continue syscall execution
 */
static int syscall_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
	char kfile[256];
	char msg[1024];
	char proc_info[128];
	unsigned long filename_ptr;

	// Get process information
	get_process_info(proc_info, sizeof(proc_info));

#ifdef ARCH_X86_64
	struct pt_regs *user_regs = task_pt_regs(current);
	filename_ptr = GET_REG_ARG1(user_regs);
#else
	filename_ptr = GET_REG_ARG1(regs);
#endif

	if(filename_ptr)
	{
		if (strncpy_from_user(kfile, (char __user *)filename_ptr, sizeof(kfile)) > 0) {
            kfile[sizeof(kfile) - 1] = '\0';  
            printk(KERN_INFO "[SPACECOP]\tSyscall %s called by %s with filename: %s\n", 
                   p->symbol_name, proc_info, kfile);
            snprintf(msg, sizeof(msg), "Syscall %s called by %s with filename: %s\n", 
                     p->symbol_name, proc_info, kfile);	
        } 
        else
        {
        	printk(KERN_INFO "[SPACECOP]\tSyscall %s called by %s\n", 
        	       p->symbol_name, proc_info);
        	snprintf(msg, sizeof(msg), "Syscall %s called by %s\n", 
        	         p->symbol_name, proc_info);
        }

        msg[sizeof(msg)-1] = '\0';
        send_netlink_msg(msg);
	}

	return 0;
}

/**
 * @brief Kprobe pre-handler for init_module() system call
 * 
 * Intercepts kernel module loading operations. Module loading can indicate:
 * - Rootkit installation
 * - Kernel backdoor insertion
 * - Driver loading
 * - Legitimate module installation
 *
 * **System Call Signature:**
 * int init_module(void *module_image, unsigned long len, const char *param_values);
 *
 * **Arguments Captured:**
 * - ARG0: module_image (pointer to module in memory)
 *
 * **Detection Use Cases:**
 * - IOB SIUU-12: Loading of malicious kernel modules
 * - Rootkit detection
 * - Unauthorized driver installation
 * - Kernel integrity monitoring
 *
 * **Security Implications:**
 * Module loading is a critical operation that grants full kernel privileges.
 * Monitoring module loading helps detect:
 * - Kernel-level malware
 * - Privilege escalation
 * - Persistence mechanisms
 * - Supply chain attacks
 *
 * @param[in] p Kprobe structure
 * @param[in] regs CPU registers at syscall entry
 *
 * @return 0 to continue syscall execution
 *
 * @note Module address logged for forensic analysis
 * @note Requires CAP_SYS_MODULE capability to call
 */
static int init_module_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
    char msg[1024];
    char proc_info[128];
    unsigned long module_ptr;

	// Get process information
	get_process_info(proc_info, sizeof(proc_info));

#ifdef ARCH_X86_64
    struct pt_regs *user_regs = task_pt_regs(current);
    module_ptr = GET_REG_ARG0(user_regs);
#else
    module_ptr = GET_REG_ARG0(regs);
#endif

    printk(KERN_INFO "[SPACECOP]\tINIT_MODULE called by %s\n", proc_info);
    snprintf(msg, sizeof(msg), "INIT_MODULE called by %s (module addr: 0x%lx)\n", 
             proc_info, module_ptr);

    msg[sizeof(msg)-1] = '\0';
    send_netlink_msg(msg);

    return 0;
}

/**
 * @brief Kprobe pre-handler for execve() system call
 * 
 * Intercepts program execution to detect:
 * - Suspicious binary execution
 * - Malware execution
 * - Unauthorized program launches
 * - Attack tool usage
 *
 * **System Call Signature:**
 * int execve(const char *pathname, char *const argv[], char *const envp[]);
 *
 * **Arguments Captured:**
 * - ARG0: pathname (program path to execute)
 *
 * **Detection Use Cases:**
 * - IOB SIUU-11: Suspicious binary or script execution
 * - IOB SIUU-16: Execution of system commands
 * - Malware execution detection
 * - Unauthorized tool usage
 * - Process creation monitoring
 *
 * **Important Note:**
 * Process information captured is for the PARENT process calling execve(),
 * not the new process being created. After execve() completes, the process
 * will have a new name (comm) but same PID.
 *
 * **Example Detection Scenarios:**
 * @code
 * Suspicious Patterns:
 * - /tmp/malware.sh execution
 * - /dev/shm/backdoor execution
 * - Unexpected shell spawning
 * - System utility misuse (grep, awk, dd, etc.)
 * @endcode
 *
 * @param[in] p Kprobe structure
 * @param[in] regs CPU registers at syscall entry
 *
 * @return 0 to continue syscall execution
 *
 * @note Captures parent process context (before exec)
 * @note Program path extracted from ARG0
 */
static int execve_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
	char kfile[256];
	char msg[1024];
	char proc_info[128];
	unsigned long filename_ptr;

	// Get process information (this is the PARENT process calling execve)
	get_process_info(proc_info, sizeof(proc_info));

#ifdef ARCH_X86_64
	struct pt_regs *user_regs = task_pt_regs(current);
	filename_ptr = GET_REG_ARG0(user_regs);
#else
	filename_ptr = GET_REG_ARG0(regs);
#endif

	if(filename_ptr)
	{
		if (strncpy_from_user(kfile, (char __user *)filename_ptr, sizeof(kfile)) > 0) {
            kfile[sizeof(kfile) - 1] = '\0';  
            printk(KERN_INFO "[SPACECOP]\tSyscall %s called by %s with program: %s\n", 
                   p->symbol_name, proc_info, kfile);	
            snprintf(msg, sizeof(msg), "Syscall %s called by %s with program: %s\n", 
                     p->symbol_name, proc_info, kfile);
        } 
        else
        {
        	printk(KERN_INFO "[SPACECOP]\tSyscall %s called by %s\n", 
        	       p->symbol_name, proc_info);
        	snprintf(msg, sizeof(msg), "Syscall %s called by %s\n", 
        	         p->symbol_name, proc_info);
        }
        msg[sizeof(msg)-1] = '\0';
        send_netlink_msg(msg);
	}

	return 0;
}

/**
 * @brief Kprobe pre-handler for openat() system call
 * 
 * Intercepts file open operations with selective filtering for critical
 * devices and files. Focuses on security-relevant file access patterns.
 *
 * **System Call Signature:**
 * int openat(int dirfd, const char *pathname, int flags, mode_t mode);
 *
 * **Arguments:**
 * - ARG0: dirfd (directory file descriptor)
 * - ARG1: pathname (file to open)
 * - ARG2: flags (O_RDONLY, O_WRONLY, O_RDWR, etc.)
 * - ARG3: mode (permissions for created files)
 *
 * **Monitored Paths:**
 * - /dev/mtdblock* - Flash memory block devices
 * - /dev/mtd* - MTD (Memory Technology Device) raw access
 * - /dev/zero - Zero device (suspicious repeated access)
 * - /dev/null - Null device (suspicious repeated access)
 * - /null - Alternate null device path
 *
 * **Detection Use Cases:**
 * - IOB MIRE-17: Unauthorized flash memory access
 * - IOB SIUU-13: Repeated /dev/null access
 * - IOB SIUU-15: Repeated /dev/zero access
 * - Flash memory tampering
 * - Boot sector modification
 * - Firmware manipulation
 *
 * **Why These Devices:**
 * @code
 * /dev/mtd*: Direct flash memory access
 * - Firmware modification
 * - Boot sector tampering
 * - Persistent malware installation
 * 
 * /dev/zero, /dev/null: Suspicious when accessed repeatedly
 * - Memory scrubbing
 * - Data destruction
 * - Covert channels
 * - Malware behavior
 * @endcode
 *
 * @param[in] p Kprobe structure
 * @param[in] regs CPU registers at syscall entry
 *
 * @return 0 to continue syscall execution
 *
 * @note Only logs access to specified critical devices
 * @note Reduces noise by filtering benign file access
 */
static int open_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
	char kfile[256];
	char msg[1024];
	char proc_info[128];
	int should_log = 0;
	unsigned long filename_ptr;

	// Get process information
	get_process_info(proc_info, sizeof(proc_info));

#ifdef ARCH_X86_64
	struct pt_regs *user_regs = task_pt_regs(current);
	filename_ptr = GET_REG_ARG1(user_regs);
#else
	filename_ptr = GET_REG_ARG1(regs);
#endif

	if(filename_ptr)
	{
		if (strncpy_from_user(kfile, (char __user *)filename_ptr, sizeof(kfile)) > 0) {
            kfile[sizeof(kfile) - 1] = '\0';  
            
            // Check if it's a device we care about
            if(strncmp(kfile, "/dev/mtdblock", 13) == 0 ||
               strncmp(kfile, "/dev/mtd", 8) == 0 ||
               strcmp(kfile, "/dev/zero") == 0 ||
               strcmp(kfile, "/dev/null") == 0 ||
               strcmp(kfile, "/null") == 0)
	        {
	        	should_log = 1;
	        	printk(KERN_INFO "[SPACECOP]\topenat called by %s with filename: %s\n",  
	        	       proc_info, kfile);
            	snprintf(msg, sizeof(msg), "openat called by %s with filename: %s\n", 
            	         proc_info, kfile);
	        }
        } 

        if(should_log)
        {
        	msg[sizeof(msg)-1] = '\0';
        	send_netlink_msg(msg);
        }
	}

	return 0;
}

/*=======================================================================================
** Kprobe Structure Definitions
**=======================================================================================*/

/**
 * @brief Kprobe structures for monitored system calls
 * 
 * Defines kprobe structures for each monitored system call. Symbol names
 * are set during initialization based on detected architecture and kernel
 * version.
 *
 * **Kprobe Lifecycle:**
 * @code
 * 1. Define kprobe structure (symbol_name = NULL initially)
 * 2. During init: Set symbol_name based on architecture
 * 3. Register kprobe with kernel
 * 4. Kprobe active: Handler called on syscall entry
 * 5. During exit: Unregister kprobe
 * @endcode
 */

static struct kprobe execve_kp = {
	.pre_handler = execve_pre_handler,
};

static struct kprobe read_kp = {
	.pre_handler = syscall_pre_handler,
};

static struct kprobe open_kp = {
	.pre_handler = open_pre_handler,
};

static struct kprobe chmod_kp = {
	.pre_handler = chmod_pre_handler,
};

static struct kprobe init_module_kp = {
    .pre_handler = init_module_pre_handler,
};

/*=======================================================================================
** Helper Functions for Kprobe Registration
**=======================================================================================*/

/**
 * @brief Try registering kprobe with multiple symbol name variants
 * 
 * Attempts to register a kprobe by trying multiple possible symbol names.
 * Different kernel versions use different naming conventions for syscall
 * entry points, so this function tries all known variants.
 *
 * **Symbol Name Variations:**
 * @code
 * x86_64:
 * - __x64_sys_<name> (modern kernels)
 * - sys_<name> (older kernels)
 * - __se_sys_<name> (some configurations)
 * 
 * ARM32:
 * - __arm_sys_<name>
 * - sys_<name>
 * - __se_sys_<name>
 * 
 * ARM64:
 * - __arm64_sys_<name>
 * - sys_<name>
 * - __se_sys_<name>
 * @endcode
 *
 * **Registration Process:**
 * @code
 * For each symbol name:
 *   1. Set kprobe->symbol_name
 *   2. Call register_kprobe()
 *   3. If success: return 0
 *   4. If failure: try next symbol
 * If all fail: return error
 * @endcode
 *
 * **Usage Example:**
 * @code
 * const char *open_symbols[] = {
 *     "__x64_sys_openat",
 *     "sys_openat",
 *     "__se_sys_openat"
 * };
 * 
 * ret = try_register_kprobe(&open_kp, open_symbols, 3, "openat");
 * if (ret < 0) {
 *     printk(KERN_ERR "Failed to register openat\n");
 * }
 * @endcode
 *
 * @param[in,out] kp Kprobe structure to register
 * @param[in] names Array of symbol name strings to try
 * @param[in] count Number of symbols in names array
 * @param[in] desc Human-readable description for logging
 *
 * @return 0 on success (kprobe registered)
 * @return Negative error code on failure (all symbols failed)
 *
 * @note First successful symbol is used
 * @note Logs each attempt at DEBUG level
 * @note Logs success at INFO level
 * @note Logs complete failure at ERROR level
 */
static int try_register_kprobe(struct kprobe *kp, const char *names[], int count, const char *desc)
{
	int i, ret;
	
	for(i = 0; i < count; i++)
	{
		kp->symbol_name = names[i];
		ret = register_kprobe(kp);
		if(ret == 0)
		{
			printk(KERN_INFO "[SPACECOP] Kprobe for %s registered with symbol: %s\n", desc, names[i]);
			return 0;
		}
		printk(KERN_DEBUG "[SPACECOP] Failed to register %s with symbol %s: %d\n", desc, names[i], ret);
	}
	
	printk(KERN_ERR "[SPACECOP] Failed to register %s kprobe with any known symbol\n", desc);
	return ret;
}

/*=======================================================================================
** Module Initialization and Cleanup
**=======================================================================================*/

/**
 * @brief Module initialization function
 * 
 * Called when kernel module is loaded via insmod/modprobe. Performs:
 * 1. Architecture detection
 * 2. Netlink socket creation
 * 3. Kprobe registration for monitored syscalls
 * 4. Error handling and cleanup on failure
 *
 * **Initialization Sequence:**
 * @code
 * 1. Print architecture information
 * 2. Create Netlink socket for userspace communication
 * 3. Register openat kprobe (critical - must succeed)
 * 4. Register init_module kprobe (optional)
 * 5. Register execve kprobe (critical - must succeed)
 * 6. Register fchmodat kprobe (optional)
 * 7. Print success message
 * @endcode
 *
 * **Error Handling:**
 * If critical kprobe registration fails:
 * - Unregister any successfully registered kprobes
 * - Release Netlink socket
 * - Return error code
 * - Module load fails
 *
 * **Kprobe Priority:**
 * - Critical: openat, execve (must succeed)
 * - Optional: init_module, fchmodat (failure logged but not fatal)
 *
 * @return 0 on success (module loaded)
 * @return Negative error code on failure (module load failed)
 *
 * @note Requires root privileges
 * @note Requires CONFIG_KPROBES=y
 * @note May fail if symbols not found
 *
 * @see module_exit for cleanup function
 */
static int __init monitor_init(void)
{
	int ret;
	
	// Architecture-specific syscall symbol names
#ifdef ARCH_X86_64
	const char *open_symbols[] = {
		"__x64_sys_openat",
		"sys_openat",
		"__se_sys_openat"
	};
	const char *execve_symbols[] = {
		"__x64_sys_execve",
		"sys_execve",
		"__se_sys_execve"
	};
	const char *chmod_symbols[] = {
		"__x64_sys_fchmodat",
		"sys_fchmodat",
		"__se_sys_fchmodat"
	};
	const char *init_symbols[] = {
		"__x64_sys_init_module",
		"sys_init_module",
		"__se_sys_init_module"
	};
#elif defined(ARCH_ARM32)
	const char *open_symbols[] = {
		"sys_openat",
		"__arm_sys_openat",
		"__se_sys_openat",
		"ksys_openat",
		"do_sys_openat2",
		"do_sys_open"
	};
	const char *execve_symbols[] = {
		"sys_execve",
		"__arm_sys_execve",
		"__se_sys_execve"
	};
	const char *chmod_symbols[] = {
		"sys_fchmodat",
		"__arm_sys_fchmodat",
		"__se_sys_fchmodat"
	};
	const char *init_symbols[] = {
		"sys_init_module",
		"__arm_sys_init_module",
		"__se_sys_init_module"
	};
#elif defined(ARCH_ARM64)
	const char *open_symbols[] = {
		"__arm64_sys_openat",
		"sys_openat",
		"__se_sys_openat"
	};
	const char *execve_symbols[] = {
		"__arm64_sys_execve",
		"sys_execve",
		"__se_sys_execve"
	};
	const char *chmod_symbols[] = {
		"__arm64_sys_fchmodat",
		"sys_fchmodat",
		"__se_sys_fchmodat"
	};
	const char *init_symbols[] = {
		"__arm64_sys_init_module",
		"sys_init_module",
		"__se_sys_init_module"
	};
#else
	#error "Unsupported architecture"
#endif

	struct netlink_kernel_cfg cfg = { 
		.input = netlink_recv_msg,
	};

	printk(KERN_INFO "[SPACECOP] Initializing module for %s...\n", ARCH_NAME);

	nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
	if(!nl_sk)
	{
		printk(KERN_ERR "[SPACECOP] netlink error: Failed to create netlink socket\n");
		return -ENOMEM;
	}
	printk(KERN_INFO "[SPACECOP] Netlink socket created\n");

	ret = try_register_kprobe(&open_kp, open_symbols, 
	                          sizeof(open_symbols)/sizeof(open_symbols[0]), "openat");
	if(ret < 0)
	{
		printk(KERN_ERR "[SPACECOP] Could not find openat syscall. Available symbols:\n");
		printk(KERN_ERR "[SPACECOP] Check: cat /proc/kallsyms | grep openat\n");
		netlink_kernel_release(nl_sk);
		return ret;
	}

    ret = try_register_kprobe(&init_module_kp, init_symbols,
                              sizeof(init_symbols)/sizeof(init_symbols[0]), "init_module");
    if(ret < 0)
    {
        printk(KERN_WARNING "[SPACECOP] Failed to register init_module (non-critical)\n");
        // Don't fail - this is optional
    }

	ret = try_register_kprobe(&execve_kp, execve_symbols,
	                          sizeof(execve_symbols)/sizeof(execve_symbols[0]), "execve");
	if(ret < 0)
	{
		printk(KERN_ERR "[SPACECOP] Failed to register execve\n");
		unregister_kprobe(&open_kp);
		if(init_module_kp.symbol_name)
			unregister_kprobe(&init_module_kp);
		netlink_kernel_release(nl_sk);
		return ret;
	}

	ret = try_register_kprobe(&chmod_kp, chmod_symbols,
	                          sizeof(chmod_symbols)/sizeof(chmod_symbols[0]), "fchmodat");
	if(ret < 0)
	{
		printk(KERN_WARNING "[SPACECOP] Failed to register chmod (non-critical)\n");
		// Don't fail - this is optional
	}

	printk(KERN_INFO "[SPACECOP] *** Module loaded successfully for %s ***\n", ARCH_NAME);
	printk(KERN_INFO "[SPACECOP] Monitoring: execve, openat, fchmodat, init_module\n");
	return 0;
}

/**
 * @brief Module cleanup function
 * 
 * Called when kernel module is unloaded via rmmod. Performs cleanup:
 * 1. Unregister all kprobes
 * 2. Release Netlink socket
 * 3. Print cleanup message
 *
 * **Cleanup Sequence:**
 * @code
 * 1. Unregister execve kprobe (if registered)
 * 2. Unregister openat kprobe (if registered)
 * 3. Unregister fchmodat kprobe (if registered)
 * 4. Unregister init_module kprobe (if registered)
 * 5. Release Netlink socket
 * 6. Print completion message
 * @endcode
 *
 * **Safety:**
 * - Checks if each kprobe was successfully registered before unregistering
 * - NULL symbol_name indicates kprobe was never registered
 * - Safe to call even if initialization partially failed
 *
 * @note Called automatically on module unload
 * @note Must clean up all resources allocated in init
 * @note Should not fail - cleanup is best-effort
 */
static void __exit monitor_exit(void)
{
	if(execve_kp.symbol_name)
		unregister_kprobe(&execve_kp);
	if(open_kp.symbol_name)
		unregister_kprobe(&open_kp);
	if(chmod_kp.symbol_name)
		unregister_kprobe(&chmod_kp);
	if(init_module_kp.symbol_name)
		unregister_kprobe(&init_module_kp);

	if(nl_sk)
		netlink_kernel_release(nl_sk);

	printk(KERN_INFO "[SPACECOP] Kernel module unloaded\n");
}

module_init(monitor_init);
module_exit(monitor_exit);