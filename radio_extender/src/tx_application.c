#include "tx_api.h"
#include "my_tasks.h"

void tx_application_define(void *first_unused_memory)
{
    (void)first_unused_memory;
    tasks_init();
}
