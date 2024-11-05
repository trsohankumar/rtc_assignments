#include "driver/gpio.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <inttypes.h>
#include <stdio.h>

// helpful to use seconds as the default time unit
#define mainTASK_CHATTERBOX_OUTPUT_FREQUENCY pdMS_TO_TICKS(1000UL)

// specifies LED blinking rate when tasks are executing
#define mainBLINK_PER_TICK pdMS_TO_TICKS(50UL)

// Constants to specify the GPIO assignment of each task
#define mainTASK_CHATTERBOX_TASK1_GPIO GPIO_NUM_16
#define mainTASK_CHATTERBOX_TASK2_GPIO GPIO_NUM_17
#define mainTASK_CHATTERBOX_TASK3_GPIO GPIO_NUM_18
#define GPIO_PIN_BIT_MASK                                                      \
  ((1ULL << mainTASK_CHATTERBOX_TASK1_GPIO) |                                  \
   (1ULL << mainTASK_CHATTERBOX_TASK2_GPIO) |                                  \
   (1ULL << mainTASK_CHATTERBOX_TASK3_GPIO))

/* TaskParams specifies the behavior of each task and is later passed to
 * chatterbox_task. The values of release_time, execution_time, and period are
 * given in seconds. The field repetitions provides the means to limit the
 * number of executions for a given task. gpio should use the above assignment
 * for its dedicated task. elapsed_time keeps track how many time units the task
 * is executed (initialize as 0).
 */
typedef struct {
  char id;
  TickType_t release_time;
  TickType_t execution_time;
  TickType_t period;
  unsigned int repetitions;
  gpio_num_t gpio;
  TickType_t elapsed_time;
} TaskParams;

/* Helper function to emulate some work for a given task.
 * During execution, the configured LED is blinking at the rate specified by
 * mainBLINK_PER_TICK.
 */
SemaphoreHandle_t useless_load_semaphore;
static void useless_load(TaskParams *params, TickType_t duration) {
  TickType_t i;
  unsigned long j;

  for (i = 0; i < duration; ++i) {
    xSemaphoreTake(useless_load_semaphore, portMAX_DELAY);
    TickType_t next_wake_time = xTaskGetTickCount();
    // TODO SWITCH BACK ELAPSED TIME TO ELAPSED TIME + 1
    printf("EXEC: Task %d (%ld/%ld)\n", params->id, params->elapsed_time,
           params->execution_time);
    for (j = 0; j < mainBLINK_PER_TICK; ++j) {
      vTaskDelayUntil(&next_wake_time, mainTASK_CHATTERBOX_OUTPUT_FREQUENCY /
                                           (2 * mainBLINK_PER_TICK));
      gpio_set_level(params->gpio, 0);
      vTaskDelayUntil(&next_wake_time, mainTASK_CHATTERBOX_OUTPUT_FREQUENCY /
                                           (2 * mainBLINK_PER_TICK));
      gpio_set_level(params->gpio, 1);
    }
    params->elapsed_time++;
    xSemaphoreGive(useless_load_semaphore);
  }
}

static void chatterbox_task(void *v_params) {
  TaskParams *params = (TaskParams *)v_params;

  for(unsigned int i = 0; i < params->repetitions;i++) {
    // printf("Start: Task %d (%d/%ld)\n", params->id, params->gpio,
    //        params->release_time);
    // printf("Inside while\n,");
    // gpio_set_level(params->gpio ,1);
    // vTaskDelay(params->release_time);
    // printf("After delay\n");
    // // delaying task till its release time
    // useless_load(params, params->execution_time);
    // printf("after useless_load\n");
    // params->repetitions--;
    // gpio_set_level(params->gpio ,0);
    // if (params->repetitions == 0 && params->id == 3) {
    //   break;
    // }
    TickType_t startTime = xTaskGetTickCount();
    vTaskDelay((params->release_time * mainTASK_CHATTERBOX_OUTPUT_FREQUENCY) / portTICK_PERIOD_MS);
    gpio_set_level(params->gpio, 1);
    useless_load(params, params->execution_time);
    gpio_set_level(params->gpio, 0);
    TickType_t endTime = xTaskGetTickCount();
    printf("%ld", ((params->release_time * mainTASK_CHATTERBOX_OUTPUT_FREQUENCY) / portTICK_PERIOD_MS) - (endTime - startTime));
    vTaskDelay(((params->release_time * mainTASK_CHATTERBOX_OUTPUT_FREQUENCY) / portTICK_PERIOD_MS) - (endTime - startTime));
  }

  vTaskDelete(NULL);
}

  /*--------------------------------------------------------------------
   * Subtask 1: Specify the task behavior.
   *
   * The implementation should satisfy the following:
   * - The task does not start its execution before its *release_time*.
   *   You may find vTaskDelay helpful, as documented here
   *   https://www.freertos.org/Documentation/02-Kernel/04-API-references/02-Task-control/01-vTaskDelay
   * - In each period, the task should execute for the duration specified
   *   by *execution_time* (in seconds). You may find *useless_load* helpful.
   * - The task should run as many periods as specified by *repetitions*.
   * - When the task is ready for execution, the LED should be in a steady 
   *   ON state. Otherwise, it should be OFF. You may find gpio_set_level
   *   helpful as documented here:
   *   https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/gpio.html#_CPPv414gpio_set_level10gpio_num_t8uint32_t
   ---------------------------------------------------------------------*/

/*--------------------------------------------------------------------
 * Subtask 2: Initialize TaskParams for each task.
 * We provide an exemplary initialization for Task 1:
 *
 * #define mainTASK_CHATTERBOX_TASK1_PRIORITY (tskIDLE_PRIORITY + 1)
 * TaskParams task1_params = {.id = 1,
 *                            .repetitions = -1,
 *                            .release_time = 1,
 *                            .execution_time = 1,
 *                            .period = 3,
 *                            .gpio = mainTASK_CHATTERBOX_TASK1_GPIO};
 *
---------------------------------------------------------------------*/
//settting prioritiy of task 1 to 3
#define mainTASK_CHATTERBOX_TASK1_PRIORITY (tskIDLE_PRIORITY + 3)
//setting priority of task 2 to 2
#define mainTASK_CHATTERBOX_TASK2_PRIORITY (tskIDLE_PRIORITY + 2)
//setting priority of task 3 to 11
#define mainTASK_CHATTERBOX_TASK3_PRIORITY (tskIDLE_PRIORITY + 1)


// defing the various task parameters as in table
TaskParams task1_params = { .id = 1,
                            .repetitions = -1,
                            .release_time = 1,
                            .execution_time = 1,
                            .period = 3,
                            .gpio = mainTASK_CHATTERBOX_TASK1_GPIO
                          };

TaskParams task2_params = { .id = 2,
                            .repetitions = -1,
                            .release_time = 0,
                            .execution_time = 2,
                            .period = 4,
                            .gpio = mainTASK_CHATTERBOX_TASK2_GPIO
                          };
TaskParams task3_params = { .id = 3,
                            .repetitions = 3,
                            .release_time = 0,
                            .execution_time = 1,
                            .period = 6,
                            .gpio = mainTASK_CHATTERBOX_TASK3_GPIO
                          };

// setting up the task handles
TaskHandle_t task1_handle;
TaskHandle_t task2_handle;
TaskHandle_t task3_handle;

void app_main(void) {
  // gpio configuration
  gpio_config_t io_conf = {.pin_bit_mask = GPIO_PIN_BIT_MASK,
                           .mode = GPIO_MODE_OUTPUT,
                           .pull_up_en = 0,
                           .pull_down_en = 0,
                           .intr_type = GPIO_INTR_DISABLE};
  gpio_config(&io_conf);

  // setting up the return type of the tasks
  BaseType_t task_status;

  // semaphore to ensure tasks are only preemted after a timeslot
  useless_load_semaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(useless_load_semaphore);

  /*--------------------------------------------------------------------
   * Subtask 3: Create the three task instances.
   * You may find the following documentation helpfull
   * https://www.freertos.org/Documentation/02-Kernel/04-API-references/01-Task-creation/01-xTaskCreate
   ---------------------------------------------------------------------*/

  printf("Nothing to see yet\n");

  task_status = xTaskCreate(chatterbox_task, "Red Led Task", 2048, &task1_params, mainTASK_CHATTERBOX_TASK1_PRIORITY, &task1_handle);

  configASSERT(task_status == pdPASS);

  task_status = xTaskCreate(chatterbox_task, "Green Led Task", 2048, &task2_params, mainTASK_CHATTERBOX_TASK2_PRIORITY, &task2_handle);

  configASSERT(task_status == pdPASS);

  task_status = xTaskCreate(chatterbox_task, "Blue Led Task", 2048, &task3_params, mainTASK_CHATTERBOX_TASK3_PRIORITY, &task3_handle);

  configASSERT(task_status == pdPASS);

}
