#include <CUnit/Basic.h>
#include <string.h>
#include "rmt_server.h"
#include "rmt_agent.h"

void testVersion(void)
{
    // Check the version's format
    char *server_version = rmt_server_version();
    char *agent_version = rmt_agent_version();
    int version_len = strlen(server_version);
    int ret = 0;
    int dot_number = 0;
    for (int i = 0; i < version_len; i++) {
        // version should only contain dot and number. 
        if (server_version[i] != '.' && !(server_version[i] >= '0' && server_version[i] <= '9')) {
            ret = 1;
            break;
        }
        // count dot number.
        if (server_version[i] == '.') {
            dot_number++;
            // dot should not be the first or the last character.
            if (i == 0 || i == version_len) {
                ret = 2;
                break;
            }
        }
    }
    // server and agent version should be the same
    CU_ASSERT_TRUE(strcmp(server_version, agent_version) == 0);
    // dot number should be 2
    CU_ASSERT_TRUE(dot_number == 2);
    // check the format
    CU_ASSERT_TRUE(ret == 0);
}

CU_TestInfo testcases_basic[] = {
    {"Version", testVersion},
    CU_TEST_INFO_NULL
};
