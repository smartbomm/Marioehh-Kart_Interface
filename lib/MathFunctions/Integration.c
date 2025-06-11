#include "Integration.h"
#include <stdio.h>

struct integration
{
    int32_t value;
    int32_t last_value;
    uint32_t last_time;
};

integration_handler_t integration_create(void)
{
    struct integration *f = (struct integration *)malloc(sizeof(struct integration));
    return f;
}

void integration_init(integration_handler_t f, int32_t value, uint32_t time)
{
    if (f != NULL)
    {
        f->last_time = time;
        f->value = 0;
        f->last_value = value;
    }
}

uint8_t integration_delete(integration_handler_t f)
{
    if (f != NULL)
    {
        free(f);
        return 1; // success
    }
    return 0; // failure
}

void integration_update(integration_handler_t f, int32_t value, uint32_t time)
{
    uint32_t dt = time - f->last_time;
    int32_t dx = value - f->last_value;
    f->value += ((dx * dt) / 2 + f->last_value * dt);
    f->last_time = time;
    f->last_value = value;
}

uint32_t integration_get_value(integration_handler_t f)
{
    struct integration *g = (struct integration *)f;
    return g->value;
}

void integration_reset(integration_handler_t f)
{
    f->value = 0;
    f->last_time = 0;
}
void integration_set_value(integration_handler_t f, int32_t value)
{
    f->value = value;
}