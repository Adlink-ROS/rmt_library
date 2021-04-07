#include <stdio.h>
#include <CUnit/Basic.h>
#include "rmt_server.h"
#include "rmt_agent.h"

extern CU_TestInfo testcases_basic[];

int suite_init(void)
{
    rmt_server_init();
    rmt_agent_init(NULL);
    return 0;
}
int suite_clean(void)
{
    rmt_server_deinit();
    rmt_agent_deinit();
    return 0;
}
void suite_setup(void)
{
    return;
}
void suite_teardown(void)
{
    return;
}

CU_SuiteInfo suites[] = {
    {"Basic Function", suite_init, suite_clean, suite_setup, suite_teardown, testcases_basic},
    CU_SUITE_INFO_NULL
};

int main()
{
    /* Test Registry */
    CU_initialize_registry();

    /* Register Suite */
    if (CUE_SUCCESS != CU_register_suites(suites)) {
        printf("Unable to register suites.\n");
        return -1;
    }

    /* Run basic test */
    CU_basic_run_tests();
    int success_numbers = CU_get_number_of_successes();
    int failure_numbers = CU_get_number_of_failures();
    printf("Total success: %d\n", success_numbers);
    printf("Total fail: %d\n", failure_numbers);

    /* Clean Registry */
    CU_cleanup_registry();

    return failure_numbers;
}