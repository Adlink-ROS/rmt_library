#ifndef _RMT_SERVER_
#define _RMT_SERVER_

#include <stdint.h>

/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C" {
#endif
/* *INDENT-ON* */

/**
 * @file
 * RMT server library API.
 */

// RMT_TODO: use char[] instead of char* in the future, or it'll cause memory leak.
typedef struct _device_info {
    unsigned long deviceID;
    char *model;
    char *host;
    char *ip;
    char *mac;
    char *rmt_version;
} device_info;

#define CONFIG_KEY_STR_LEN 1023
typedef struct _data_info {
    unsigned long deviceID;
    char value_list[CONFIG_KEY_STR_LEN + 1];
} data_info;

typedef struct _transfer_result {
    int result;
    void *pFile;
    uint32_t file_len;
} transfer_result;

typedef enum _transfer_status {
    TRANSFER_DONE = 0,
    TRANSFER_RUNNING,
    SERVER_ERROR,
    AGENT_ERROR,
} transfer_status;

/**
 * @brief rmt_server_config
 * Set the config for RMT server
 *
 * @param[in] interface The network interface
 *
 * @returns The error code
 * @retval  0 Success
 * @retval -1 Something wrong
 */
int rmt_server_config(char *interface);

/**
 * @brief rmt_server_init
 * Init RMT server
 *
 * @returns The error code
 * @retval  0 Success
 * @retval -1 Something wrong
 */
int rmt_server_init(void);

/**
 * @brief rmt_server_create_device_list
 * Create device list RMT server received.
 *
 * @param[out] num How many devices return
 *
 * @returns device_info: Array of device info.
 */
device_info *rmt_server_create_device_list(int *num);

/**
 * @brief rmt_server_free_device_list
 * Free the device list RMT server created.
 *
 * @param[in] dev The device list created by rmt_server_create_device_list
 *
 * @returns The error code
 * @retval  0 Success
 * @retval -1 Something wrong
 */
int rmt_server_free_device_list(device_info *dev);

/**
 * @brief rmt_server_get_info
 * Get the information from device.
 *
 * @param[in]  id_list unsigned long array
 * @param[in]  id_num: the number of id
 * @param[in]  key_list: the string of key and each key is splitted with ";", for example "cpu;ram;..."
 * @param[out] info_num: the number of return data_info
 *
 * @returns data_info: data_info array. value_list is the string of key-value pairs, for example "cpu:20;ram:30;...".
 */
data_info* rmt_server_get_info(unsigned long *id_list, int id_num, char *key_list, int *info_num);
int rmt_server_free_info(data_info* info_list);

/**
 * @brief rmt_server_set_info
 * Set the information to device
 *
 * @param[in]  data_info: data_info array. value_list is the string of key-value pairs, for example "hostname:my_name;locate:on;..."
 * @param[in]  dev_num: the number of device
 * @param[out] info_num: the number of return data_info
 *
 * @returns data_info: data_info array. value_list is the string of key-value pairs, for example "hostname:0;locate:-1;..."
 */
data_info* rmt_server_set_info(data_info *dev_list, int dev_num, int *info_num);

/**
 * @brief rmt_server_set_info_with_same_value
 * Set the information to device with the same value
 *
 * @param[in]  id_list: unsigned long array
 * @param[in]  id_num: the number of id
 * @param[in]  value_list: the pairs of key and value, for example "hostname:my_name;locate:on;..."
 * @param[out] inf_num: the number of return data_info
 *
 * @returns data_info: data_info array. value_list is the string of key-value pairs, for example "hostname:0;locate:-1;..."
 */
data_info* rmt_server_set_info_with_same_value(unsigned long *id_list, int id_num, char *value_list, int *info_num);

/**
 * @brief rmt_server_send_file
 * Send files to agent
 *
 * @param[in]  id_list: unsigned long array
 * @param[in]  id_num: the number of id
 * @param[in]  filename: the name of the file you want to send
 * @param[in]  pFile: the content of the file
 * @param[in]  file_len: the length of the file
 *
 * @returns The error code
 * @retval  0 Success
 * @retval -1 Something wrong
 */
int rmt_server_send_file(unsigned long *id_list, int id_num, char *filename, void *pFile, uint32_t file_len);

/**
 * @brief rmt_server_recv_file
 * Get files from agent
 *
 * @param[in]  id: the id you want to get file from
 * @param[in]  filename: the name of the file you want to receive
 *
 * @returns The error code
 * @retval  0 Success
 * @retval -1 Something wrong
 */
int rmt_server_recv_file(unsigned long id, char *filename);

/**
 * @brief rmt_server_get_result
 * Get the result of file transfer by handle ID
 * Note that the status of handle_id will be clear when you get the DONE status.
 *
 * @param[in]  device_id: the device ID
 * @param[out] result: the file transfer result
 *
 * @returns transfer_status indicates the file transfer status of device ID.
 */
transfer_status rmt_server_get_result(unsigned long device_id, transfer_result *result);

/**
 * @brief rmt_server_deinit
 * Deinit the RMT library after use.
 *
 * @returns The error code
 * @retval  0 Success
 * @retval -1 Something wrong
 */
int rmt_server_deinit(void);

/**
 * @brief rmt_server_version
 * Return the version of RMT library
 *
 * @returns return the library version in string
 */
char* rmt_server_version(void);

/* *INDENT-OFF* */
#ifdef __cplusplus
} /* extern "C" */
#endif 
/* *INDENT-ON* */

#endif /*_RMT_SERVER_*/
