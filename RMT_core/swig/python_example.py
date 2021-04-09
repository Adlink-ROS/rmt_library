#!/usr/bin/env python3
import rmt_py_wrapper
import json

def search():
    rmt_py_wrapper.rmt_server_init()

    num_ptr = rmt_py_wrapper.new_intptr()
    dev_list = rmt_py_wrapper.device_info_list.frompointer(rmt_py_wrapper.rmt_server_create_device_list(num_ptr))
    num = rmt_py_wrapper.intptr_value(num_ptr)
    rmt_py_wrapper.delete_intptr(num_ptr) # release num_ptr

    # Put data in JSON format    
    data=[{"Total": num}]
    for i in range(0, num):
        item = {
            "ID": dev_list[i].deviceID,
            "Model": dev_list[i].model,
            "Host": dev_list[i].host,
            "IP": dev_list[i].ip,
            "MAC": dev_list[i].mac,
            "RMT_VERSION": dev_list[i].rmt_version
        }
        data.append(item)
    result = json.dumps(data, indent=4)
    print(result)

    # put search ID into array
    id_list = rmt_py_wrapper.ulong_array(num)
    for i in range(0, num):
        id_list[i] = dev_list[i].deviceID
    info_num_ptr = rmt_py_wrapper.new_intptr()
    info_list = rmt_py_wrapper.data_info_list.frompointer(rmt_py_wrapper.rmt_server_get_info(id_list, num, "cpu", info_num_ptr))
    info_num = rmt_py_wrapper.intptr_value(info_num_ptr)
    rmt_py_wrapper.delete_intptr(info_num_ptr) # release info_num_ptr
    for i in range(0, info_num):
        print("ID %d" % info_list[i].deviceID)
        print("value list: %s" % info_list[i].value_list)

def main():
    print("RMT_VERSION=%s" % rmt_py_wrapper.rmt_server_version())

if __name__ == "__main__":
    main()
    search()
