// Copyright © 2026 Aerospace Corporation
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * @file functions_syscall_kernel_module.h
 * @brief System call monitoring via Linux kernel module for SpaceCop IDS
 *
 * This header provides function prototypes and data structures for monitoring
 * system calls through a custom Linux kernel module. The system uses Netlink
 * sockets for kernel-to-userspace communication to detect:
 * - Excessive system call usage (potential exploits)
 * - Anomalous system call patterns
 * - Privilege escalation attempts
 * - System call-based attacks
 *
 * **Architecture:**
 * - Kernel module intercepts system calls and sends events to userspace
 * - Netlink socket provides communication channel
 * - Userspace component counts the events and checks thresholds
 * - Configurable tolerance levels per system call type
 * - Enable/disable monitoring per IOB (Indicator of Behavior)
 *
 * **Key Features:**
 * - Real-time system call monitoring
 * - Tolerance-based threshold detection
 * - Dynamic enable/disable of monitors
 * - Integration with IDS alerting infrastructure
 */

#ifndef _FUNCTIONS_SYSCALL_KERNEL_MODULE_H_
#define _FUNCTIONS_SYSCALL_KERNEL_MODULE_H_

/*=======================================================================================
** Required Header Files
**=======================================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/statvfs.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <openssl/evp.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <netinet/in.h>
#include <time.h>

#include "spacecop_ids_helper.h"

/*=======================================================================================
** Constants and Macros
**=======================================================================================*/

/** @brief Netlink protocol number for userspace communication */
#define NETLINK_USER 31

/** @brief Maximum payload size for Netlink messages in bytes */
#define MAX_PAYLOAD 1024

/*=======================================================================================
** Type Definitions
**=======================================================================================*/

/**
 * @brief System call monitoring configuration structure
 *
 * Tracks the monitoring state, thresholds, and current counts for a specific
 * system call or group of system calls identified by an IOB (Indicator of Behavior).
 */
typedef struct 
{
    int8_t enabled;         /**< Monitor enabled flag (0=disabled, 1=enabled) */
    char name[64];          /**< Human-readable name of the system call or monitor */
    char iob[10];           /**< IOB identifier (e.g., "CSNE-17") */
    int16_t curr_count;     /**< Current count of system calls since last check */
    int32_t tolerance;      /**< Threshold before generating alert */
} SyscallInfo;

/*=======================================================================================
** Function Prototypes
**=======================================================================================*/

/**
 * @brief Initialize the system call monitoring subsystem
 *
 * Ensures the companion kernel module is loaded. Checks /proc/modules for
 * the "aerospace" module and, if it is not present, loads it via insmod
 * from "cf/aerospace.ko".
 *
 * @return void
 *
 * @note Called by SYSCALL_Monitor() before it opens its Netlink socket
 * @note Requires root privileges to load the kernel module
 * @note Loading the module via insmod exits the program on failure
 *
 * @see SYSCALL_Monitor()
 */
void Init_Syscall(void);

/**
 * @brief Toggle monitoring on/off for a specific IOB
 *
 * Enables or disables system call monitoring for the specified IOB identifier.
 * When disabled, the monitor will not generate alerts even if thresholds are
 * exceeded.
 *
 * @param[in] iob IOB identifier string (e.g., "CSNE-17")
 *
 * @return void
 *
 * @note Requires Init_Syscall() to be called first
 * @note Toggles between enabled and disabled states
 * @note Does not affect tolerance settings
 *
 * Example usage:
 * @code
 * // Disable monitoring for CSNE-17
 * Toggle_Monitor("CSNE-17");
 *
 * // Re-enable monitoring for CSNE-17
 * Toggle_Monitor("CSNE-17");
 * @endcode
 *
 * @see Init_Syscall()
 * @see Set_Tolerance()
 */
void Toggle_Monitor(const char *iob);

/**
 * @brief Set the tolerance threshold for a specific IOB
 *
 * Configures the maximum number of system calls allowed before generating
 * an alert. The tolerance value determines how many system calls can occur
 * within a monitoring period before being considered anomalous.
 *
 * @param[in] iob IOB identifier string (e.g., "SIUU-15")
 * @param[in] tolerance Maximum allowed system call count before alert
 *
 * @return void
 *
 * @note Requires Init_Syscall() to be called first
 * @note Tolerance value must be positive
 * @note Setting tolerance to 0 will alert on any system call
 * @note Changes take effect immediately
 *
 * Example usage:
 * @code
 * // Alert if more than 100 system calls occur
 * Set_Tolerance("SIUU-15", 100);
 *
 * // Very strict: alert on any occurrence
 * Set_Tolerance("SIUU-15", 0);
 * @endcode
 *
 * @see Init_Syscall()
 * @see Toggle_Monitor()
 */
void Set_Tolerance(const char *iob, int16_t tolerance);

/**
 * @brief Run the system call monitoring loop
 *
 * Ensures the kernel module is loaded (via Init_Syscall()), creates and binds
 * a Netlink socket, registers the userspace listener with the kernel module,
 * then enters an infinite loop receiving system call events, parsing them, and
 * generating IDS alerts for enabled monitors whose detection criteria match.
 *
 * Monitoring process (per received event):
 * 1. Block on recvmsg() for the next kernel event
 * 2. Parse the raw kernel message into a structured event
 * 3. Apply detection rules (execve, openat, INIT_MODULE)
 * 4. Generate alerts for enabled monitors that match
 *
 * Alerts are sent via:
 * - IDS telemetry messages (SPACECOP_ReportIDSMsg)
 * - STIX reports (write_to_stix)
 * - cFE error events (CFE_EVS_SendEvent)
 *
 * @return void
 *
 * @note Runs an infinite loop and does not return under normal operation
 * @note Intended to run on its own dedicated task/thread
 * @note Calls Init_Syscall() itself before opening the socket
 * @note Blocks on recvmsg() waiting for kernel messages
 * @note Only exits (and returns) on a socket send/receive error
 * @note Only generates alerts for enabled monitors
 * @note Filters out noisy processes (shells, editors)
 *
 * @warning Requires the kernel module to be loaded and responsive
 *
 * @see Init_Syscall()
 * @see Toggle_Monitor()
 * @see Set_Tolerance()
 */
void SYSCALL_Monitor(void);

#endif /* _FUNCTIONS_SYSCALL_KERNEL_MODULE_H_ */