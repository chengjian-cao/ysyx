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

#include <common.h>
#include "/home/chengjian/ysyx-workbench/nemu/src/monitor/sdb/sdb.h"
void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
  // FILE *file = fopen("tools/gen-expr/build/input", "r");
  // if (file == NULL)
  // {
  //   perror("Error opening file");
  //   return 1;
  // }
  // char *line = NULL;
  // size_t len = 0;
  // __ssize_t nread;
  // /* Initialize the simple debugger. */
  
  // while ((nread = getline(&line, &len, file)) != -1){
  //   // printf("Retrieved line of length %zd:\n", nread);
  //   // Remove the newline character if present
  //   if (nread > 0 && line[nread-1] == '\n') {
  //       line[nread-1] = '\0';
  //   }
  //   char *result = strtok(line, " ");
  //   printf("%s", result);
  //   // char *expression = result + strlen(result) +1;
  //   char *expression = strtok(NULL, "\0");
  //   printf(" %s\n", expression);
  //   // printf("%c, %c\n", expression[0], expression[1]);
  //   bool *sucess = NULL;
  //   word_t eval = 0;
  //   if(expression != NULL){
  //   eval= expr(expression, sucess);}
  //   printf("eval = %u\n", eval);
  
  // }
  
  // free(line); // 释放动态分配的缓冲区
  // fclose(file);
  
  return 0;
}
