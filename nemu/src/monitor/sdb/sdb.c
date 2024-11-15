/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include "watchpoint.c"
#include "/home/chengjian/ysyx-workbench/nemu/include/memory/vaddr.h"
static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
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

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);
static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Single-step execution si [N]", cmd_si },
  { "info", "Print program status", cmd_info },
  { "x", "Scan memory", cmd_x }, //x 10 $esp
  { "p", "Expression evaluation", cmd_p}, //p $eax + 1, 0x80100000+   ($a0 +5)*4 - *(  $t1 + 8) + number
  { "w", "Set watchpoint", cmd_w}, //w *0x2000
  { "d", "Deletes the watchpoint with ID N", cmd_d},
  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

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
static int cmd_si(char *args) {
    /* extract the first argument */
  char *arg = strtok(NULL, " ");

  if(arg == NULL) {
    cpu_exec(1);
  }
  else {
    cpu_exec(atoi(arg));
  }

  return 0;
}

static int cmd_info(char *args){
  /* extract the first argument */
  char *arg = strtok(NULL, " ");

  if(strcmp(arg, "r") == 0){
    isa_reg_display();
  }else if (strcmp(arg, "w") == 0)
  {
    WP *wp = head;
    printf("head address: %p, wp address: %p\n", (void *)head, (void *)wp);

    while (wp != NULL)
    {
      printf("%s %u\n", wp->expr, wp->value);
      wp = wp->next;
    }
    
  }
  else{
    printf("Unknown command '%s'\n", arg);
  }

  return 0;
}
static int cmd_x(char *args){
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int N = 0;
  if(arg != NULL){
    N = atoi(arg);
  }
  // printf("N is %d\n", N);
  /* extract the second argument */
  arg = strtok(NULL, " ");
  uint32_t vaddr = 0;
  if (arg != NULL)
  {
    vaddr = (uint32_t)strtol(arg, NULL, 16);
    // printf("vaddr: 0x%08X\n", vaddr);
  }

  
  for (int i = 0; i < N; i++)
  {
    uint32_t read = vaddr_read(vaddr, 4);
    printf("0x%08x: 0x%08x\n", vaddr, read);  
    vaddr +=4 ;
  }
  
  return 0;
}

static int cmd_p(char *args){
  /* extract the first argument */
  char *arg = strtok(NULL, "\0");
  // printf("expression is %s\n", arg);
  bool success = false;
  word_t eval = expr(arg, &success);
  if (success == true)
  {
     printf("%u\n", eval);
  } 
  return 0;
}
static int cmd_w(char *args){
  char *arg = strtok(NULL, "\0");
  assert(arg != NULL);
  WP *wp = new_wp();
  wp->expr = strdup(arg);
  printf("head address: %p, wp address: %p\n", (void *)head, (void *)wp);

}
static int cmd_d(char *args){
  char *arg = strtok(NULL, " ");
  word_t NO = atoi(arg);
  WP *wp = head;
  while (wp != NULL)
  {
    if(wp->NO != NO){
      wp = wp->next;
    }else{
      break;
    }
  }
  printf("wp is %s\n", wp->expr);
  free_wp(wp);  
}
void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
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

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
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

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
