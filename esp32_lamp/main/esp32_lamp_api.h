#ifndef LAMP_API_H
#define LAMP_API_H

#include <stdbool.h>

#include <time.h>



typedef struct
{
    bool paired;
    bool has_task;

    char title[64];
    char description[128];

    time_t timer_end;
    time_t deadline;

    bool has_timer;
    bool has_deadline;

} LampTask;


bool api_register_lamp(
    const char *device_id
);

bool api_check_pair_status(
    const char *device_id
);


LampTask api_get_tasks(const char *device_id);

#endif