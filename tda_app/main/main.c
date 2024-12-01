#include "display.h"
#include "tasks.h"
#include <math.h>
#include <stdio.h>
#include <unistd.h>

typedef struct {
  bool accepted;
  TaskInfo task_info;
} AcceptanceTestResult;
AcceptanceTestResult default_result = {false, default_task_info};

TickType_t div_ceil(TickType_t x, TickType_t y) { return x / y + (x % y != 0); }

void utilization_bound_test(TaskParams **params, unsigned int task_id,
                            AcceptanceTestResult *result) {
  double utilization = 0;
  bool accepted = false;

  /*--------------------------------------------------------------------
   * Subtask 1: Implement the Utilization Bound Test from the lecture
   ---------------------------------------------------------------------*/
  // Utilization bound U(n) <= n(2^(1/n) - 1)
  int size_of_tasks = 3;
  double urm_bound = size_of_tasks*(pow(2, 1.0/size_of_tasks) - 1);

  for(int i = 0; i < size_of_tasks; i++){
    utilization += (params[i]->execution_time/(double) params[i]->period);
  }

  if(utilization <= urm_bound) accepted = true;

  result->accepted = accepted;
  result->task_info.util = utilization;
}

void worst_case_simulation(TaskParams **params, unsigned int task_id,
                           AcceptanceTestResult *result) {
  TickType_t completion_time = 0;
  bool accepted = false;

  /*--------------------------------------------------------------------
   * Subtask 2: Implement the Worst Case Simulation from the lecture
   ---------------------------------------------------------------------*/
  
  for(int i = 0; i < task_id; i++) {
    completion_time += (params[i]->execution_time*(params[task_id]->period/(double) params[i]->period));
  }

  if(completion_time < params[task_id]->period) accepted = true;
  

  result->accepted = accepted;
  result->task_info.wcs_result = completion_time;
}

void time_demand_analysis(TaskParams **params, unsigned int task_id,
                          AcceptanceTestResult *result) {
  TickType_t t_last = 0, t_next = 0;
  bool accepted = false;

  /*--------------------------------------------------------------------
   * Subtask 3: Implement the Time Demand Analysis from the lecture
   ---------------------------------------------------------------------*/
  while((t_last > 0 && t_next > 0) && 
        (t_last == t_next || t_next > params[task_id]->deadline)){
      t_last = t_next;

      if(t_next == 0){
        for(int i = 0; i < task_id; i++) t_next += params[i]->execution_time;
      }else{
        for(int i = 0; i < task_id; i++) t_next += params[i]->execution_time*ceil(t_last/(double) params[i]->period);
        t_next += params[task_id]->execution_time;
      }

  }

  if(t_next <= params[task_id]->deadline && t_next == t_last) accepted = true;

  result->accepted = accepted;
  result->task_info.tda_result = t_next;
}

/* Determine if params[task_id] can be scheduled.
 * - params: array of all task parameters (e.g., needed to perform TDA)
 * - task_id: index such that params[task_id] is the task under consideration
 * - results: output parameter yielding the acceptance test result */
void acceptance_test(TaskParams **params, unsigned int task_id,
                     AcceptanceTestResult *result) {

  /*--------------------------------------------------------------------
   * Subtask 4: Call the above acceptance tests in a suitable order.
   *  In particular, recall which of these tests are necessary,
   *  sufficient, or both.
   *  Ensure that the final value of result->accepted is true if and
   *  only if the task encoded by params[task_id] can be scheduled.
  //  ---------------------------------------------------------------------*/
  utilization_bound_test(params, task_id, result);
  if(result->accepted == true) return;

  worst_case_simulation(params, task_id, result);
  if(result->accepted == true) return;

  // time_demand_analysis(params, task_id, result);
  if(result->accepted == true) return;

  result->accepted = false;
}

/* Tasks are scheduled according to RMA, i.e.,
 * prio(task1) > prio(task2) > prio(task3)
 * No need to change anything here... */
TaskParams task1_params = {.id = 1,
                           .execution_time = 2,
                           .period = 4,
                           .deadline = 4,
                           .gpio = mainTASK_TASK1_GPIO};
TaskParams task2_params = {.id = 2,
                           .execution_time = 2,
                           .period = 7,
                           .deadline = 7,
                           .gpio = mainTASK_TASK2_GPIO};
TaskParams task3_params = {.id = 3,
                           .execution_time = 1,
                           .period = 8,
                           .deadline = 8,
                           .gpio = mainTASK_TASK3_GPIO};

void app_main(void) {
  /* No need to change anything here... */
  task_setup();
  ssd1306_setup();

  TaskParams *task_set[3] = {&task1_params, &task2_params, &task3_params};
  AcceptanceTestResult results[3] = {default_result, default_result,
                                     default_result};

  // check acceptance test and store decision in results array
  for (unsigned int i = 0; i < 3; i++) {
    acceptance_test(task_set, i, &results[i]);
    if (!results[i].accepted)
      continue;

    // if accepted, create the task
    char task_name[6];
    snprintf(task_name, 6, "task%c", task_set[i]->id);
    xTaskCreate((void *)task_implementation, task_name,
                configMINIMAL_STACK_SIZE + 256, task_set[i],
                tskIDLE_PRIORITY + 3 - i, NULL);
  }
  printf("bavck to add main");
  // print acceptance test results
  for (;;) {
    for (unsigned int i = 0; i < 3; i++) {
      ssd1306_print_task_info(task_set[i], &results[i].task_info);
      sleep(1);
    }
  }
}
