#include <CUnit/Basic.h>
#include <string.h>
#include "rmt_server.h"

void testVersion(void)
{
    // Check the version's format
    char *version = rmt_lib_version();
    int version_len = strlen(version);
    int ret = 0;
    int dot_number = 0;
    for (int i = 0; i < version_len; i++) {
        // version should only contain dot and number. 
        if (version[i] != '.' && !(version[i] >= '0' && version[i] <= '9')) {
            ret = 1;
            break;
        }
        // count dot number.
        if (version[i] == '.') {
            dot_number++;
            // dot should not be the first or the last character.
            if (i == 0 || i == version_len) {
                ret = 2;
                break;
            }
        }
    }
    // dot number should be 2
    CU_ASSERT_TRUE(dot_number == 2);
    // check the format
    CU_ASSERT_TRUE(ret == 0);
}

CU_TestInfo testcases_basic[] = {
    {"Version", testVersion},
    CU_TEST_INFO_NULL
};
