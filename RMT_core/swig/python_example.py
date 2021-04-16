#!/usr/bin/env python3
import rmt_py_wrapper
import json

def config(dev_list, dev_num):
    # Create config key string
    config_list = ["cpu", "ram", "hostname", "wifi"]
    config_key_str = ""
    for item in config_list:
        config_key_str += item + ';'

    # Get device info list
    id_list = rmt_py_wrapper.ulong_array(dev_num)
    for i in range(0, dev_num):
        id_list[i] = dev_list[i].deviceID
    info_num_ptr = rmt_py_wrapper.new_intptr()
    info_list = rmt_py_wrapper.data_info_list.frompointer(rmt_py_wrapper.rmt_server_get_info(id_list, dev_num, config_key_str, info_num_ptr))
    info_num = rmt_py_wrapper.intptr_value(info_num_ptr)
    rmt_py_wrapper.delete_intptr(info_num_ptr) # release info_num_ptr
    
    print("=== get config result ===")
    config_data = []
    for i in range(0, info_num):
        # Split the result string into dictionary data
        result_list = info_list[i].value_list.split(";")
        dict_data = {"deviceID": info_list[i].deviceID}
        for item in result_list:
            for key in config_list:
                if key in item:
                    dict_data[key] = item[len(key)+1:]
        # print(dict_data)
        config_data.append(dict_data)
    result = json.dumps(config_data, indent=4)
    print(result)

    # Set device info list
    info_num_ptr = rmt_py_wrapper.new_intptr()
    data_info_array = rmt_py_wrapper.new_data_info_array(1)
    data_info_element = rmt_py_wrapper.data_info()
    data_info_element.deviceID = 6166
    data_info_element.value_list = "locate:on"
    rmt_py_wrapper.data_info_array_setitem(data_info_array, 0, data_info_element)
    rmt_py_wrapper.rmt_server_set_info(data_info_array, 1, info_num_ptr)
    info_num = rmt_py_wrapper.intptr_value(info_num_ptr)
    rmt_py_wrapper.delete_intptr(info_num_ptr) # release info_num_ptr

    return config_data
def search():
    rmt_py_wrapper.rmt_server_init()

    num_ptr = rmt_py_wrapper.new_intptr()
    dev_list = rmt_py_wrapper.device_info_list.frompointer(rmt_py_wrapper.rmt_server_create_device_list(num_ptr))
    num = rmt_py_wrapper.intptr_value(num_ptr)
    rmt_py_wrapper.delete_intptr(num_ptr) # release num_ptr

    # Put data in JSON format    
    data = {"total": num, "items": []}
    items = []
    for i in range(0, num):
        item = {
            "ID": dev_list[i].deviceID,
            "Model": dev_list[i].model,
            "Host": dev_list[i].host,
            "IP": dev_list[i].ip,
            "MAC": dev_list[i].mac,
            "RMT_VERSION": dev_list[i].rmt_version
        }
        items.append(item)

    print("=== search result ===")
    data["items"] = items
    result = json.dumps(data, indent=4)
    print(result)
    return dev_list, num

def main():
    print("RMT_VERSION=%s" % rmt_py_wrapper.rmt_server_version())
    dev_list, num = search()
    config(dev_list, num)

if __name__ == "__main__":
    main()
