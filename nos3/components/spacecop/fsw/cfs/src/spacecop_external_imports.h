// Copyright © 2026 Aerospace Corporation
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * @file spacecop_external_imports.h
 * @brief External application interface definitions for SpaceCop IDS
 *
 * This header aggregates message ID and message structure definitions from
 * external cFS applications that SpaceCop monitors. These imports enable
 * SpaceCop to:
 * - Subscribe to telemetry from monitored applications
 * - Parse command and telemetry messages
 * - Validate message structures
 * - Detect unauthorized or anomalous commands
 *
 * **Monitored Applications:**
 * - Generic Thruster: Spacecraft propulsion control
 * - CFE Time Services: Time synchronization and management
 * - TO Lab: Telemetry output application
 * - CAM: Camera control application
 * - CFE Executive Services: Core cFE services
 *
 * **Usage:**
 * SpaceCop uses these definitions to:
 * 1. Subscribe to application message IDs via Software Bus
 * 2. Parse received messages in monitoring functions
 * 3. Validate command parameters against expected ranges
 * 4. Detect anomalous command sequences
 * 5. Generate IDS alerts for policy violations
 *
 * **Message ID Categories:**
 * - Command MIDs: Ground commands to applications
 * - Telemetry MIDs: Status and housekeeping data
 * - Request MIDs: Service request messages
 *
 * **Function Codes:**
 * Each application defines function codes (command codes) that specify
 * the operation to perform within a command message.
 *
 * @note These headers are from external applications - do not modify
 * @note Changes to monitored applications may require SpaceCop updates
 * @note Message structures must match monitored application versions
 *
 * @see spacecop_table_defs.h for monitor configuration
 */

/*=======================================================================================
** Generic Thruster Application Interface
**
** Provides interface to spacecraft thruster control application for monitoring
** propulsion commands and telemetry.
**=======================================================================================*/

/**
 * @brief Generic Thruster message IDs
 * 
 * Defines Software Bus Message IDs for the Generic Thruster application:
 * - GENERIC_THRUSTER_CMD_MID: Ground commands (fire thrusters, configure)
 * - GENERIC_THRUSTER_HK_TLM_MID: Housekeeping telemetry
 * - GENERIC_THRUSTER_STATUS_TLM_MID: Thruster status data
 * 
 * **Monitoring Use Cases:**
 * - Detect unauthorized thruster firing commands
 * - Monitor thruster usage patterns
 * - Validate command parameters (duration, force)
 * - Detect replay attacks on propulsion commands
 */
#include "generic_thruster_msgids.h"

/**
 * @brief Generic Thruster message structures
 * 
 * Defines message payload structures for the Generic Thruster application:
 * - Command message structures (fire, configure, enable/disable)
 * - Telemetry message structures (status, housekeeping)
 * - Parameter data types and ranges
 * 
 * **Monitoring Use Cases:**
 * - Parse thruster fire commands to extract parameters
 * - Validate thruster selection (valid thruster IDs)
 * - Check firing duration against safety limits
 * - Monitor accumulated thruster usage
 */
#include "generic_thruster_msg.h"

/*=======================================================================================
** CFE Time Services Interface
**
** Provides interface to cFE time management services for monitoring time
** synchronization commands.
**=======================================================================================*/

/**
 * @brief CFE Time Services message IDs
 * 
 * Defines Software Bus Message IDs for CFE Time Services:
 * - CFE_TIME_CMD_MID: Time management commands
 * - CFE_TIME_HK_TLM_MID: Time services housekeeping
 * - CFE_TIME_DIAG_TLM_MID: Time diagnostic data
 * 
 * **Monitoring Use Cases:**
 * - Detect unauthorized time updates
 * - Monitor time synchronization commands
 * - Detect time manipulation attacks
 * - Validate time source selection
 * 
 * @note Time manipulation can affect mission-critical operations
 */
#include "cfe_time_msgids.h"

/**
 * @brief CFE Time Services function codes
 * 
 * Defines command codes for CFE Time Services operations:
 * - CFE_TIME_NOOP_CC: No operation
 * - CFE_TIME_RESET_COUNTERS_CC: Reset telemetry counters
 * - CFE_TIME_SET_TIME_CC: Set spacecraft time
 * - CFE_TIME_SET_MET_CC: Set Mission Elapsed Time
 * - CFE_TIME_SET_STCF_CC: Set Spacecraft Time Correlation Factor
 * - CFE_TIME_SET_LEAP_SECONDS_CC: Set leap seconds
 * - CFE_TIME_ADD_ADJUST_CC: Add time adjustment
 * - CFE_TIME_SUB_ADJUST_CC: Subtract time adjustment
 * - And others...
 * 
 * **Monitoring Use Cases:**
 * - Detect unauthorized time set commands
 * - Monitor time adjustment frequency
 * - Validate time source changes
 * - Detect time-based attack patterns
 */
#include "cfe_time_fcncodes.h"

/**
 * @brief CFE Time Services message structures
 * 
 * Defines message payload structures for CFE Time Services:
 * - Time set command structures
 * - Time adjustment structures
 * - Time diagnostic telemetry
 * 
 * **Monitoring Use Cases:**
 * - Parse time set commands to extract new time value
 * - Validate time adjustments are within reasonable bounds
 * - Monitor time source selection changes
 */
#include "cfe_time_msg.h"

/*=======================================================================================
** TO Lab (Telemetry Output) Application Interface
**
** Provides interface to telemetry output application for monitoring telemetry
** routing configuration.
**=======================================================================================*/

/**
 * @brief TO Lab message IDs
 * 
 * Defines Software Bus Message IDs for TO Lab application:
 * - TO_LAB_CMD_MID: TO Lab commands (enable output, add/remove subscriptions)
 * - TO_LAB_HK_TLM_MID: TO Lab housekeeping telemetry
 * 
 * **Monitoring Use Cases:**
 * - Detect unauthorized telemetry output configuration changes
 * - Monitor telemetry subscription additions/removals
 * - Detect attempts to suppress IDS alert telemetry
 * - Validate telemetry routing changes
 * 
 * @note Attackers may attempt to disable IDS telemetry output
 */
#include "to_lab_msgids.h"

/*=======================================================================================
** CAM (Camera) Application Interface
**
** Provides interface to camera control application for monitoring imaging
** commands.
**=======================================================================================*/

/**
 * @brief CAM message IDs
 * 
 * Defines Software Bus Message IDs for CAM application:
 * - CAM_CMD_MID: Camera commands (capture, configure, download)
 * - CAM_HK_TLM_MID: Camera housekeeping telemetry
 * - CAM_IMAGE_TLM_MID: Image data telemetry
 * 
 * **Monitoring Use Cases:**
 * - Detect unauthorized image capture commands
 * - Monitor camera configuration changes
 * - Validate image download requests
 * - Detect excessive camera usage (resource exhaustion)
 */
#include "cam_msgids.h"

/*=======================================================================================
** CFE Executive Services Interface
**
** Provides interface to cFE Executive Services for monitoring application
** lifecycle commands.
**=======================================================================================*/

/**
 * @brief CFE Executive Services function codes
 * 
 * Defines command codes for CFE Executive Services operations:
 * - CFE_ES_NOOP_CC: No operation
 * - CFE_ES_RESET_COUNTERS_CC: Reset telemetry counters
 * - CFE_ES_RESTART_CC: Restart cFE
 * - CFE_ES_START_APP_CC: Start an application
 * - CFE_ES_STOP_APP_CC: Stop an application
 * - CFE_ES_RESTART_APP_CC: Restart an application
 * - CFE_ES_RELOAD_APP_CC: Reload an application
 * - CFE_ES_SET_MAX_PR_COUNT_CC: Set max processor resets
 * - CFE_ES_DELETE_CDS_CC: Delete Critical Data Store
 * - CFE_ES_DUMP_CDS_REGISTRY_CC: Dump CDS registry
 * - CFE_ES_WRITE_SYSLOG_CC: Write system log to file
 * - CFE_ES_CLEAR_SYSLOG_CC: Clear system log
 * - CFE_ES_WRITE_ER_LOG_CC: Write exception/reset log
 * - CFE_ES_CLEAR_ER_LOG_CC: Clear exception/reset log
 * - And others...
 * 
 * **Monitoring Use Cases:**
 * - Detect unauthorized application start/stop commands
 * - Monitor application restart attempts
 * - Detect cFE restart commands (potential attack)
 * - Validate file system operations
 * - Detect attempts to clear logs (cover tracks)
 * 
 * @note These are high-privilege commands that should be tightly controlled
 */
#include "cfe_es_fcncodes.h"

/**
 * @brief CFE Executive Services message IDs
 * 
 * Defines Software Bus Message IDs for CFE Executive Services:
 * - CFE_ES_CMD_MID: Executive Services commands
 * - CFE_ES_HK_TLM_MID: Executive Services housekeeping
 * - CFE_ES_APP_TLM_MID: Application information telemetry
 * - CFE_ES_MEMSTATS_TLM_MID: Memory statistics telemetry
 * 
 * **Monitoring Use Cases:**
 * - Monitor application lifecycle commands
 * - Detect unauthorized system management operations
 * - Validate file system access patterns
 * - Monitor memory pool operations
 */
#include "cfe_es_msgids.h"

/**
 * @brief CFE Executive Services message structures
 * 
 * Defines message payload structures for CFE Executive Services:
 * - Application control command structures (start, stop, restart, reload)
 * - File operation command structures (delete, move, copy)
 * - System management command structures (reset, dump logs)
 * - Telemetry structures (app info, memory stats)
 * 
 * **Monitoring Use Cases:**
 * - Parse app start commands to validate application path
 * - Validate app stop/restart commands against policy
 * - Monitor file operation commands for suspicious activity
 * - Detect attempts to manipulate system logs
 * 
 * @note These structures contain critical system management parameters
 */
#include "cfe_es_msg.h"

/*=======================================================================================
** End of External Imports
**=======================================================================================*/