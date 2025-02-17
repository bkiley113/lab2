#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t i32;

struct process {
    u32 pid;
    u32 arrival_time;
    u32 burst_time;

    TAILQ_ENTRY(process) pointers;

    /* Additional fields here */

    u32 remaining_time;
    u32 start_time;       // when it first gets CPU
    u32 completion_time;  // when it finishes
    bool has_started;
    u32 last_executed;

    /* End of "Additional fields here" */
};

TAILQ_HEAD(process_list, process);

u32 next_int(const char **data, const char *data_end)
{
  u32 current = 0;
  bool started = false;
  while (*data != data_end)
  {
    char c = **data;

    if (c < 0x30 || c > 0x39)
    {
      if (started)
      {
        return current;
      }
    }
    else
    {
      if (!started)
      {
        current = (c - 0x30);
        started = true;
      }
      else
      {
        current *= 10;
        current += (c - 0x30);
      }
    }

    ++(*data);
  }

  printf("Reached end of file while looking for another integer\n");
  exit(EINVAL);
}

u32 next_int_from_c_str(const char *data)
{
  char c;
  u32 i = 0;
  u32 current = 0;
  bool started = false;
  while ((c = data[i++]))
  {
    if (c < 0x30 || c > 0x39)
    {
      exit(EINVAL);
    }
    if (!started)
    {
      current = (c - 0x30);
      started = true;
    }
    else
    {
      current *= 10;
      current += (c - 0x30);
    }
  }
  return current;
}

void init_processes(const char *path,
                    struct process **process_data,
                    u32 *process_size)
{
  int fd = open(path, O_RDONLY);
  if (fd == -1)
  {
    int err = errno;
    perror("open");
    exit(err);
  }

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    int err = errno;
    perror("stat");
    exit(err);
  }

  u32 size = st.st_size;
  const char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
  {
    int err = errno;
    perror("mmap");
    exit(err);
  }

  const char *data_end = data_start + size;
  const char *data = data_start;

  *process_size = next_int(&data, data_end);

  *process_data = calloc(sizeof(struct process), *process_size);
  if (*process_data == NULL)
  {
    int err = errno;
    perror("calloc");
    exit(err);
  }

  for (u32 i = 0; i < *process_size; ++i)
  {
    (*process_data)[i].pid = next_int(&data, data_end);
    (*process_data)[i].arrival_time = next_int(&data, data_end);
    (*process_data)[i].burst_time = next_int(&data, data_end);
  }

  munmap((void *)data, size);
  close(fd);
}

/* Comparison function to sort processes by arrival_time ascending */
static int compare_by_arrival_time(const void *a, const void *b) {
    const struct process *p1 = (const struct process *)a;
    const struct process *p2 = (const struct process *)b;
    if (p1->arrival_time < p2->arrival_time) return -1;
    if (p1->arrival_time > p2->arrival_time) return 1;
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) 
    {
        return EINVAL;
    }

    struct process *data;
    u32 size;
    init_processes(argv[1], &data, &size);

    // Initialize
    u32 quantum_length = next_int_from_c_str(argv[2]);

    struct process_list ready_queue;
    TAILQ_INIT(&ready_queue);

    /* Your code here*/

    // Sort the processes by arrival time
    qsort(data, size, sizeof(struct process), compare_by_arrival_time);

    // Prepare fields
    for (u32 i = 0; i < size; i++) {
        data[i].remaining_time = data[i].burst_time;
        data[i].has_started = false;
        data[i].completion_time = 0;
        data[i].start_time = 0;
        data[i].last_executed = 0;  // optional for incremental waiting
    }

    // We'll track how many processes have completed
    u32 completed = 0;
    u32 current_time = 0;
    u32 next_arrival_index = 0;

    // For final averages
    // We will compute waiting_time = completion_time - arrival_time - burst_time
    // response_time = start_time - arrival_time (when it first starts)
    float total_waiting_time = 0.0f;
    float total_response_time = 0.0f;

    // Keep running until all processes complete
    while (completed < size) {

        // 1) Add all processes that arrive at or before current_time
        while (next_arrival_index < size &&
               data[next_arrival_index].arrival_time <= current_time) {
            TAILQ_INSERT_TAIL(&ready_queue, &data[next_arrival_index], pointers);
            next_arrival_index++;
        }

        // 2) If the ready queue is empty but we still have processes to come,
        //    jump time to the arrival of the next process.
        if (TAILQ_EMPTY(&ready_queue)) {
            if (next_arrival_index < size) {
                // Jump to the arrival time of the next process
                current_time = data[next_arrival_index].arrival_time;
            } else {
                // No processes left to arrive, but maybe some haven't finished
                // (unlikely if the queue is empty, but just in case)
                break;
            }
            continue; // re-check arrivals after jumping
        }

        // 3) Pop the front of the queue
        struct process *curr_proc = TAILQ_FIRST(&ready_queue);
        TAILQ_REMOVE(&ready_queue, curr_proc, pointers);

        // If first time running, record response time
        if (!curr_proc->has_started) {
            curr_proc->has_started = true;
            curr_proc->start_time = current_time;
            total_response_time += (curr_proc->start_time - curr_proc->arrival_time);
        }

        // 4) Run the process for up to quantum_length (or until it finishes).
        //    We also need to handle arrivals that might happen mid-quantum.
        u32 time_slice = quantum_length;
        if (curr_proc->remaining_time < time_slice) {
            time_slice = curr_proc->remaining_time;
        }

        // Instead of jumping current_time all at once,
        // we'll increment it step by step to catch new arrivals.
        // (This is the easiest way to handle "arrivals during a time slice".)
        u32 run_for = 0;
        while (run_for < time_slice) {
            curr_proc->remaining_time--;
            current_time++;
            run_for++;

            // Check if any new process arrives exactly at current_time
            while (next_arrival_index < size &&
                   data[next_arrival_index].arrival_time <= current_time) {
                TAILQ_INSERT_TAIL(&ready_queue, &data[next_arrival_index], pointers);
                next_arrival_index++;
            }

            if (curr_proc->remaining_time == 0) {
                // This process finished
                curr_proc->completion_time = current_time;
                completed++;
                break;  // no need to continue this time slice
            }
        }

        // If not finished, re-insert it into the queue
        if (curr_proc->remaining_time > 0) {
            TAILQ_INSERT_TAIL(&ready_queue, curr_proc, pointers);
        }
    }

    // Now compute waiting times from completion times
    for (u32 i = 0; i < size; i++) {
        u32 turnaround_time = data[i].completion_time - data[i].arrival_time;
        u32 waiting_time = turnaround_time - data[i].burst_time;
        total_waiting_time += waiting_time;
    }

    // Print results
    float avg_waiting = total_waiting_time / (float)size;
    float avg_response = total_response_time / (float)size;

    /* End of "Your code here" */

    printf("Average waiting time: %.2f\n", avg_waiting);
    printf("Average response time: %.2f\n", avg_response);

    free(data);
    return 0;
}
