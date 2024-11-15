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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough 65536
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static int check_zero(int n){
  if(buf[n] == '/'){
    return 1;
  }else if(buf[n] == '('){
     return check_zero(n - 1);
  }else if(n <= 0){
    return 0;
  }
  else{
    return 0;
  }
}

static uint32_t choose(uint32_t n) {
    uint32_t re = rand() % n;
    // int pre = (int)(strlen(buf) - 1);
    // if (re == 0 &&  pre >= 0)
    // {   
    //   if (check_zero(pre))
    //   {
    //     return choose(n);
    //   }                    
    // }    
    return re;
}

static void gen_num() {
  char num_str[2];
  num_str[0] = '0' + choose(10);
  while (num_str[0] == '0'){
    int pre = (int)strlen(buf) - 1;
    if(check_zero(pre)){
      num_str[0] = '0' + choose(10);
    }else{
      break;
    }
  }
  num_str[1] = '\0';

  if (strlen(buf) + strlen(num_str) < sizeof(buf)){
    strcat(buf, num_str);
    // printf("the size of buf is %lu in gen_num.\n", strlen(buf));
    // printf("num_str is %s\n", num_str);
  }
  else return;
        
}

static void gen(int s) {
  char str[2] = {s, '\0'};
  if (strlen(buf) + strlen(str) < sizeof(buf)){
    strcat(buf, str);
    // printf("the size of buf is %lu in gen.\n", strlen(buf));
    // printf("str is %s\n", str);
  }else return;
  
} 

static void gen_rand_op() {
  uint32_t c = '0';
  switch (choose(4))
  {
  case 0:
    c = '+';
    break;
  case 1:
    c = '-';
    break;
  case 2:
    c = '*';
    break;
  case 3:
    c = '/';
    break;
  default:
    break;
  }
  gen(c);
}
static void gen_rand_expr() {
  // buf[0] = '\0';
  if (strlen(buf) >= sizeof(buf)) return;
  switch (choose(3)) {
    case 0: gen_num(); break;
    case 1: gen('('); gen_rand_expr(); gen(')'); break;
    default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    buf[0] = '\0';
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr -Werror");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
