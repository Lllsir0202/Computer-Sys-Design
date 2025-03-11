#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

// si
static int cmd_si(char *args);

// info
static int cmd_info(char *args);

// p
static int cmd_p(char *args);

// // x
static int cmd_x(char *args);

// // w
// static int cmd_w(char *args);

// // d
// static int cmd_d(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  // Followings are added
  { "si", "Exec N(default N=1) steps", cmd_si},
  { "info", "Print informations of something", cmd_info},
  { "p", "Get the result of the expr", cmd_p},
  { "x", "Scan the memory", cmd_x},
  // { "w", "Set the watchpoint", cmd_w},
  // { "d", "Delete NO.n watchpoint", cmd_d}
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args){
  char* arg = strtok(NULL, " ");
  // 这里的arg其实就是si [N] 中的n
  if(arg == NULL){
    // 说明是默认情况
    // 执行一条指令后暂停
    cpu_exec(1);
    return 0;
  }
  else{
    // 说明设定了参数
    int n = atoi(arg);
    cpu_exec(n);
    return 0;
  }
}

static int cmd_info(char *args){
  char* arg = strtok(NULL, " ");
  int i;
  // 如果是r，那么输出所有寄存器
  if(strcmp(arg, "r") == 0){
    for(i = 0 ; i < 8; i++){
      printf("%s\t",regsl[i]);
      printf("%u\n",reg_l(i));
    }
    printf("eip\t");
    printf("%u\n",cpu.eip);
  }else if(strcmp(arg, "w") == 0){

  }else{
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
  // 
}

static int cmd_p(char *args){
  char* arg = strtok(NULL, "");
  bool success;
  uint32_t result = expr(arg, &success);
  if(!success){
    // 表示没有成功
    Log("Failed to phase the expr");
    return -1;
  }
  else{
    printf("Result: %x\n", result);
  }
  return 0;
}

static int cmd_x(char *args){
  char* arg = strtok(NULL, " ");
  int N = atoi(arg);
  // 在expr时改变操作，将剩下的所有字符都得到，从而进行expr的计算
  arg = strtok(NULL, "");
  // 这里需要提取出来内存地址，由于是32位的，所以我们可以使用uint32_t存即可
  vaddr_t addr;
  addr = strtoul(arg, NULL, 16);
  // printf("0x%x\n", addr);
  int i;
  // printf("111");
  // uint8_t* ptr = (uint8_t*)addr;
  // printf("%x", (int)*ptr);
  for(i = 0 ; i < N ; i++){
    // 首先输出内存地址
    printf("0x%x\t", addr);
    for(int j = 0 ; j < 4 ; j++){
      printf("0x%x ", vaddr_read(addr,1));
      addr++;
    }
    printf("\n");
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
