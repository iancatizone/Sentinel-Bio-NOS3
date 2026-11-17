// Copyright © 2026 Aerospace Corporation
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * @file ml_interface.h
 * @brief Machine Learning bridge interface for SpaceCop IDS
 *
 * This header provides function prototypes and data structures for interfacing
 * with an external machine learning anomaly detection system. The ML bridge
 * enables SpaceCop to:
 * - Send telemetry data to ML models for analysis
 * - Receive anomaly alerts from ML models
 * - Parse and classify ML-detected anomalies
 * - Generate IDS alerts based on ML findings
 *
 * **Architecture:**
 * - TCP socket communication with ML server (localhost)
 * - Send port 9111: SpaceCop → ML server
 * - Receive port 9112: ML server → SpaceCop
 * - JSON-based alert protocol
 * - Blocking alert reception on a dedicated task
 *
 * **Supported Alert Types:**
 * - ML Anomaly: Statistical deviations detected by ML models
 * - Command Flooding: Excessive command rates
 * - Replay Attack: Duplicate command patterns
 *
 * **Key Features:**
 * - Real-time telemetry streaming to ML models
 * - JSON alert parsing
 * - IOB (Indicator of Behavior) mapping
 * - Configurable monitor enable/disable
 * - Integration with STIX reporting
 */

#ifndef ML_INTERFACE_H
#define ML_INTERFACE_H

/*=======================================================================================
** Required Header Files
**=======================================================================================*/

#include "cfe.h"
#include "cfe_sb.h"
#include <stdint.h>
#include <stdbool.h>
#include "spacecop_external_imports.h"
#include "spacecop_ids_helper.h"

/*=======================================================================================
** Constants and Macros
**=======================================================================================*/

/** @brief TCP port for sending data to ML server */
#define ML_SEND_PORT 9111

/** @brief TCP port for receiving alerts from ML server */
#define ML_RECV_PORT 9112

/** @brief IP address of ML server (localhost) */
#define ML_SERVER_IP "127.0.0.1"

/** @brief Maximum buffer size for socket communications */
#define BUFFER_SIZE 8192

/** @brief Maximum number of telemetry subscriptions for ML monitoring */
#define MAX_SUBSCRIPTIONS 50

/*=======================================================================================
** Type Definitions
**=======================================================================================*/

/**
 * @brief ML Bridge application data structure
 *
 * Contains the runtime state of the ML bridge including socket file
 * descriptors and run status.
 */
typedef struct {
    int32_t send_sockfd;    /**< Socket file descriptor for sending to ML server */
    int32_t recv_sockfd;    /**< Socket file descriptor for receiving from ML server */
    uint32_t RunStatus;     /**< Application run status flag */
} MLBridge_AppData_t;

/**
 * @brief ML monitor configuration structure
 *
 * Tracks the monitoring state and identifiers for ML-based detection
 * of specific message types.
 */
typedef struct 
{
    int8_t enabled;         /**< Monitor enabled flag (0=disabled, 1=enabled) */
    char iob[10];           /**< IOB identifier (e.g., "UACE-16") */
    char mid[64];           /**< Message ID being monitored */
} MLMonitorInfo;

/**
 * @brief Alert type enumeration
 *
 * Classifies the type of anomaly detected by the ML system.
 */
typedef enum {
    ALERT_TYPE_UNKNOWN = 0,         /**< Unknown or unparseable alert type */
    ALERT_TYPE_ML_ANOMALY,          /**< Statistical anomaly detected by ML model */
    ALERT_TYPE_COMMAND_FLOODING,    /**< Excessive command rate detected */
    ALERT_TYPE_REPLAY_ATTACK        /**< Duplicate command pattern detected */
} AlertType_t;

/**
 * @brief Parsed alert structure
 *
 * Contains structured information extracted from a JSON alert message
 * received from the ML server.
 */
typedef struct {
    AlertType_t alert_type;         /**< Type of alert */
    char severity[16];              /**< Severity level (e.g., "high", "medium", "low") */
    char stix_pattern[64];          /**< STIX pattern identifier */
    char description[256];          /**< Human-readable alert description */
    char hex[512];                  /**< Hexadecimal representation of anomalous data */
    char system[64];                /**< System or component that generated the data */
    char data_type[16];             /**< Type of data (e.g., "command", "telemetry") */
    int current_rate;               /**< Current rate (for flooding detection) */
    int threshold;                  /**< Threshold value (for flooding detection) */
    double mse;                     /**< Mean squared error (for ML anomaly) */
    char responsible_feature[128];  /**< Feature responsible for anomaly */
} ParsedAlert_t;

/*=======================================================================================
** Socket Communication Functions
**=======================================================================================*/

/**
 * @brief Initialize TCP sockets for ML communication
 *
 * Creates and configures TCP sockets for bidirectional communication with
 * the ML server. Sets up:
 * - Send socket that connects to ML_SEND_PORT on the ML server
 * - Receive socket bound to ML_RECV_PORT that listens for connections
 * - Blocking mode for reliable message delivery
 *
 * @return int32_t CFE_SUCCESS on success, error code on failure
 *
 * @note Sockets connect to ML_SERVER_IP (127.0.0.1)
 * @note Must be called before sending/receiving data
 * @note Prints diagnostic messages on failure
 *
 * @see MLBridge_Cleanup()
 */
int32_t MLBridge_InitSockets(void);

/**
 * @brief Send raw telemetry data to ML server as hexadecimal
 *
 * Converts binary telemetry data to hexadecimal string format and sends it
 * to the ML server via TCP. The ML server uses this data to train models
 * and detect anomalies.
 *
 * @param[in] data Pointer to binary telemetry data
 * @param[in] length Length of data in bytes
 * @param[in] msg_id cFE Software Bus Message ID
 *
 * @return int32_t CFE_SUCCESS on success, error code on failure
 *
 * @note Data is converted to hex string before transmission
 * @note Includes message ID in transmission for ML model routing
 * @note Blocking send operation with automatic reconnection on failure
 *
 * Example usage:
 * @code
 * uint8_t tlm_data[64];
 * CFE_SB_MsgId_t msg_id = CFE_SB_ValueToMsgId(0x0880);
 * MLBridge_SendRawHex(tlm_data, sizeof(tlm_data), msg_id);
 * @endcode
 */
int32_t MLBridge_SendRawHex(const uint8_t *data, size_t length, CFE_SB_MsgId_t msg_id);

/**
 * @brief Check for and process alerts from ML server
 *
 * Accepts a client connection and blocks receiving JSON alert messages from
 * the ML server. Each received alert is parsed and processed to generate
 * appropriate IDS alerts.
 *
 * @return void
 *
 * @note Blocking operation - runs an infinite loop and does not return
 * @note Must be run on its own dedicated task/thread
 * @note Blocks on accept() and recv() waiting for the ML server
 * @note Automatically parses and handles received alerts
 *
 * @see MLBridge_ParseAlert()
 * @see MLBridge_HandleAlert()
 */
void MLBridge_CheckForResponse(void);

/**
 * @brief Clean up ML bridge resources
 *
 * Closes socket file descriptors and releases any allocated resources.
 * Should be called during application shutdown.
 *
 * @return void
 *
 * @note Safe to call multiple times
 * @note Closes both send and receive sockets
 */
void MLBridge_Cleanup(void);

/*=======================================================================================
** Alert Parsing and Handling Functions
**=======================================================================================*/

/**
 * @brief Parse JSON alert message from ML server
 *
 * Extracts structured information from a JSON-formatted alert message
 * received from the ML server. Populates a ParsedAlert_t structure with
 * the parsed data.
 *
 * JSON format expected:
 * @code
 * {
 *   "alert_type": "ML_ANOMALY",
 *   "severity": "high",
 *   "stix_pattern": "UACE-16",
 *   "description": "Anomaly detected in telemetry",
 *   "mse": 0.95,
 *   "responsible_feature": "temperature"
 * }
 * @endcode
 *
 * @param[in] json_buffer Pointer to JSON string buffer
 * @param[out] alert Pointer to ParsedAlert_t structure to populate
 *
 * @return AlertType_t Type of alert parsed, or ALERT_TYPE_UNKNOWN on error
 *
 * @note Handles missing or malformed JSON fields gracefully
 * @note Supports multiple alert types (anomaly, flooding, replay)
 * @note Prints warnings for unparseable fields
 */
AlertType_t MLBridge_ParseAlert(const char* json_buffer, ParsedAlert_t* alert);

/**
 * @brief Handle a parsed alert by generating IDS alerts
 *
 * Processes a parsed alert structure and generates appropriate IDS alerts
 * via multiple channels:
 * - IDS telemetry messages
 * - STIX reports
 * - System log entries (via CFE_ES_WriteToSysLog)
 *
 * @param[in] alert Pointer to parsed alert structure
 *
 * @return void
 *
 * @note Alert content varies based on alert type
 * @note Includes relevant details (MSE, rates, features) in messages
 */
void MLBridge_HandleAlert(const ParsedAlert_t* alert);

/*=======================================================================================
** IOB Mapping Functions
**=======================================================================================*/

/**
 * @brief Extract IOB identifier from JSON response
 *
 * Parses a JSON response to extract the telemetry "name" field, appends
 * "_MID" to form a Message ID, and looks up the corresponding IOB in the
 * ML monitor configuration table.
 *
 * @param[in] json_buffer Pointer to JSON string buffer
 *
 * @return const char* IOB identifier string, or NULL if not found
 *
 * @note Looks for the "name" field in JSON and appends "_MID" for lookup
 * @note Returns static string - do not free
 */
const char* MLBridge_GetIOBFromResponse(const char* json_buffer);

/**
 * @brief Map alert type to IOB identifier
 *
 * Converts an AlertType_t enumeration value to the corresponding IOB
 * identifier string.
 *
 * @param[in] alert_type Alert type enumeration value
 *
 * @return const char* IOB identifier string
 *
 * Mappings:
 * - ALERT_TYPE_ML_ANOMALY → "UACE-16"
 * - ALERT_TYPE_COMMAND_FLOODING → "UACE-9"
 * - ALERT_TYPE_REPLAY_ATTACK → "UACE-7"
 * - ALERT_TYPE_UNKNOWN → NULL
 *
 * @note Returns NULL if the alert type is unknown or the monitor is disabled
 * @note Returns static string - do not free
 */
const char* MLBridge_GetIOBFromAlertType(AlertType_t alert_type);

/*=======================================================================================
** Monitor Control Functions
**=======================================================================================*/

/**
 * @brief Toggle ML monitoring on/off for a specific IOB
 *
 * Enables or disables ML-based monitoring for the specified IOB identifier.
 * When disabled, telemetry is not sent to the ML server for that IOB.
 *
 * @param[in] iob IOB identifier string (e.g., "UACE-16")
 *
 * @return void
 *
 * @note Toggles between enabled and disabled states
 * @note Does not affect already-queued telemetry
 */
void Toggle_MLMonitor(const char *iob);

#endif /* ML_INTERFACE_H */