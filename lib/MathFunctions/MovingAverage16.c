#include "MovingAverage16.h"

struct _moving_average16_t
{
    uint8_t size;
    uint8_t index;
    int32_t sum;
    int16_t *buffer;
};

moving_average_handler_t moving_average_create(uint8_t size)
{
    moving_average_handler_t f = (moving_average_handler_t)malloc(sizeof(struct _moving_average16_t));
    f->buffer = (int16_t*)malloc(size * sizeof(int16_t));
    f->size = size;
    f->index = 0;
    f->sum = 0;
    return f;
}
void moving_average_init(moving_average_handler_t f, int16_t value)
{
    for (uint8_t i = 0; i < f->size; i++)
    {
        f->buffer[i] = value;
    }
    f->sum = value * f->size;
}

void moving_average_delete(moving_average_handler_t f)
{
    if (f != NULL)
    {
        free(f->buffer);
        free(f);
    }
}
void moving_average_write(moving_average_handler_t f, int16_t value)
{
    f->sum -= f->buffer[f->index];
    f->buffer[f->index] = value;
    f->sum += value;
    f->index = (f->index + 1) % f->size;
}
int16_t moving_average_read(moving_average_handler_t f)
{
    return (int16_t)(f->sum / f->size);
}