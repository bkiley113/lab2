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
    u32 start_time;
    u32 completion_time;
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

//I use quicksort from stdlib.h, hence we need a comparison function to make it work
//this one will compare by arrival time
static int compare_arrival_time(const void *a, const void *b) 
{
  const struct process *proc1 = (const struct process *)a;
  const struct process *proc2 = (const struct process *)b;

  if (proc1->arrival_time < proc2->arrival_time) return -1;
  if (proc1->arrival_time > proc2->arrival_time) return 1;
  return 0;
}

int main(int argc, char *argv[]) 
{
  if (argc != 3) 
  {
      return EINVAL;
  }

  struct process *data;
  u32 size;
  init_processes(argv[1], &data, &size);

  // Initialize
  u32 quantum_length = next_int_from_c_str(argv[2]);

  struct process_list list;
  TAILQ_INIT(&list);

  u32 total_waiting_time = 0;
  u32 total_response_time = 0;

  /* Your code here*/

  //first, ensure that processes are sorted by arrival time using quicksort
  qsort(data, size, sizeof(struct process), compare_arrival_time);

  //here are the initial additional values for each process in 'data'
  for (u32 i = 0; i < size; i++) {
      data[i].remaining_time = data[i].burst_time;
      data[i].has_started = false;
      data[i].completion_time = 0;
      data[i].start_time = 0;
      data[i].last_executed = 0;
  }

  //track if we have processes every job or not
  u32 completed = 0;

  //gives us a relative marker to define milestones for each process
  u32 current_time = 0;

  //we will use this to step through each process in 'data'
  u32 next_arrival_index = 0;

  //time slice for the 'execution' part
  u32 time_slice = 0;

  //introduce this metric to step throught the execution time of each process
  u32 run_for = 0;

  //turnaround time for each process, used in final calculations
  u32 turnaround_time = 0;


  while (completed < size) {
    //first, we will add to the queue jobs that are scheduled at or before current time
    while ((next_arrival_index < size) && (data[next_arrival_index].arrival_time <= current_time)) {

      //insert them at the end
        TAILQ_INSERT_TAIL(&list, &data[next_arrival_index], pointers);
        next_arrival_index++;
    }

    //if the queue is empty, then we will jump to the next scheduled job
    if (TAILQ_EMPTY(&list)){
        //if there is an incoming job, jump to it
        if (next_arrival_index < size) {
            current_time = data[next_arrival_index].arrival_time;
        } else {
            //if not, we are done scheduling and we bail out of the loop
            break;
        }
        //now we move on to the next loop iteration to pick up those processes we jumped to
        continue;
    }

    //once we have done some populating, we begin to 'execute' processes off the list
    //start by popping off the top of the queue!
    struct process *curr_proc = TAILQ_FIRST(&list);
    TAILQ_REMOVE(&list, curr_proc, pointers);

    //if the current process has not been started yet, this is where we get out response time metric
    //so we update that accordingly, and set its start time as well
    if (!curr_proc->has_started){
        curr_proc->has_started = true;
        curr_proc->start_time = current_time;
        total_response_time += (curr_proc->start_time - curr_proc->arrival_time);
    }

    //now we determine its time slice by the remaining time in the job it has less
    //we do this to differentiate between taking a whole quantum or only part of one
    time_slice = quantum_length;
    if (curr_proc->remaining_time < time_slice) {
        time_slice = curr_proc->remaining_time;
    }

    //here, we implement a loop that will simulate the running of a process for its time slice
    //if our run_for counter reaches time slice, we continue on through the loop
    //if it doesn't, we get to record that the process finished
    run_for = 0;
    //we do this in a one-at-a-time-unit fashion
    while (run_for < time_slice) {
        curr_proc->remaining_time--;
        current_time++;
        run_for++;

        //here, we consider other processes that may be set to arrive because we are still incrementing time and they need attention
        while ((next_arrival_index < size) && (data[next_arrival_index].arrival_time <= current_time)) {
            //put them at the back of the queue as usual
            TAILQ_INSERT_TAIL(&list, &data[next_arrival_index], pointers);
            next_arrival_index++;
        }
        //if we finish a process...
        if (curr_proc->remaining_time == 0) {
            curr_proc->completion_time = current_time;  //we record its completion time
            completed++;   //as well as the number of completions
            break;        //and jump out of our execution loop! onto the next
        }
    }
    //this is if we didn't finish running the process, so we add it back and rinse/repeat
    if (curr_proc->remaining_time > 0) {
        TAILQ_INSERT_TAIL(&list, curr_proc, pointers);
    }
  }

  //now we add the turnaround time of each process up and then we get the total waiting time!
  for (u32 i = 0; i < size; i++) {
      turnaround_time = data[i].completion_time - data[i].arrival_time;
      total_waiting_time += turnaround_time - data[i].burst_time;
  }


  /* End of "Your code here" */

  printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
  printf("Average response time: %.2f\n", (float)total_response_time / (float)size);

  free(data);
  return 0;
}
