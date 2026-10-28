#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
    
#define BUFFER_SIZE 8

typedef struct
{
    uint8_t buffer[BUFFER_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} RingBuffer;

/* Initialize the ring buffer */
void ringbuf_init(RingBuffer *rb)
{
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;

    memset(rb->buffer, 0, BUFFER_SIZE);
}

/* Check whether buffer is empty */
int ringbuf_is_empty(const RingBuffer *rb)
{
    return rb->count == 0;
}

/* Check whether buffer is full */
int ringbuf_is_full(const RingBuffer *rb)
{
    return rb->count == BUFFER_SIZE;
}

/* Return number of bytes currently stored */
uint8_t ringbuf_count(const RingBuffer *rb)
{
    return rb->count;
}

/* Write one byte into the buffer */
int ringbuf_write(RingBuffer *rb, uint8_t data)
{
    if (ringbuf_is_full(rb))
    {
        return -1;
    }

    rb->buffer[rb->head] = data;

    /*
     * BUFFER_SIZE is 8, which is a power of 2.
     *
     * head & (BUFFER_SIZE - 1) is equivalent to:
     * head % BUFFER_SIZE
     *
     * Bitwise AND is faster on many MCUs without a
     * hardware divider because it avoids division.
     */
    rb->head = (rb->head + 1U) & (BUFFER_SIZE - 1U);

    rb->count++;

    return 0;
}

/* Read one byte from the buffer */
int ringbuf_read(RingBuffer *rb, uint8_t *data)
{
    if (ringbuf_is_empty(rb))
    {
        return -1;
    }

    *data = rb->buffer[rb->tail];

    /*
     * Since BUFFER_SIZE is a power of 2, bitwise AND
     * provides efficient circular wrap-around.
     */
    rb->tail = (rb->tail + 1U) & (BUFFER_SIZE - 1U);

    rb->count--;

    return 0;
}

int main(void)
{
    RingBuffer rb;
    uint8_t data;
    uint8_t write_data[] =
    {
        0x41, 0x42, 0x43, 0x44,
        0x45, 0x46, 0x47, 0x48
    };

    uint8_t new_data[] =
    {
        0x49, 0x4A, 0x4B
    };

    size_t i;

    /* Initialize buffer */
    ringbuf_init(&rb);

    printf("=== Ring Buffer Test ===\n\n");

    /* Step 1: Write 8 bytes */
    printf("Step 1: Filling buffer\n");

    for (i = 0; i < sizeof(write_data); i++)
    {
        if (ringbuf_write(&rb, write_data[i]) == 0)
        {
            printf("[WRITE] 0x%02X -> OK (count=%u)",
                   write_data[i],
                   ringbuf_count(&rb));

            if (ringbuf_is_full(&rb))
            {
                printf(" FULL");
            }

            printf("\n");
        }
        else
        {
            printf("[WRITE] 0x%02X -> FAIL (buffer full)\n",
                   write_data[i]);
        }
    }

    printf("\nBuffer full: %s\n", ringbuf_is_full(&rb) ? "YES" : "NO");
    printf("Count: %u\n\n", ringbuf_count(&rb));

    /* Step 2: Attempt to write when full */
    printf("Step 2: Write when buffer is full\n");

    if (ringbuf_write(&rb, 0x99) != 0)
    {
        printf("[WRITE] 0x99 -> FAIL (buffer full)\n");
    }

    printf("\n");

    /* Step 3: Read 3 bytes */
    printf("Step 3: Read 3 bytes\n");

    for (i = 0; i < 3; i++)
    {
        if (ringbuf_read(&rb, &data) == 0)
        {
            printf("[READ] -> 0x%02X (count=%u)\n",
                   data,
                   ringbuf_count(&rb));
        }
        else
        {
            printf("[READ] -> FAIL (buffer empty)\n");
        }
    }

    printf("Count after 3 reads: %u\n\n", ringbuf_count(&rb));

    /* Step 4: Write 3 new bytes */
    printf("Step 4: Reusing freed slots\n");

    for (i = 0; i < sizeof(new_data); i++)
    {
        if (ringbuf_write(&rb, new_data[i]) == 0)
        {
            printf("[WRITE] 0x%02X -> OK (count=%u)",
                   new_data[i],
                   ringbuf_count(&rb));

            if (ringbuf_is_full(&rb))
            {
                printf(" FULL");
            }

            printf("\n");
        }
        else
        {
            printf("[WRITE] 0x%02X -> FAIL (buffer full)\n",
                   new_data[i]);
        }
    }

    printf("Count after new writes: %u\n\n", ringbuf_count(&rb));

    /* Step 5: Read all 8 remaining bytes */
    printf("Step 5: Read remaining bytes\n");

    while (!ringbuf_is_empty(&rb))
    {
        if (ringbuf_read(&rb, &data) == 0)
        {
            printf("[READ] -> 0x%02X (count=%u)\n",
                   data,
                   ringbuf_count(&rb));
        }
    }

    printf("\nBuffer empty: %s\n",
           ringbuf_is_empty(&rb) ? "YES" : "NO");

    printf("Count: %u\n\n", ringbuf_count(&rb));

    /* Step 6: Attempt to read from empty buffer */
    printf("Step 6: Read when buffer is empty\n");

    if (ringbuf_read(&rb, &data) != 0)
    {
        printf("[READ] (empty) -> FAIL (buffer empty)\n");
    }

    return 0;
}