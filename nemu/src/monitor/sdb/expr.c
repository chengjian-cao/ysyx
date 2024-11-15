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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM, TK_HEX, TK_REG, TK_NEQ, TK_AND, TK_DEREF

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {"0x[0-9a-fA-F]+", TK_HEX},
  {"[0-9]+", TK_NUM},   //number
  {"\\-", '-'},
  {"\\*", '*'},
  {"/", '/'},
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},
  {"&&", TK_AND},
  {"\\(", '('},
  {"\\)", ')'},
  {"\\$[0a-f][0-9]+", TK_REG},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[65536] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;
  int nr_token_local = 0;
  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
        
        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //     i, rules[i].regex, position, substr_len, substr_len, substr_start);

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        position += substr_len;
        
        switch (rules[i].token_type) {
          case TK_NUM: 
          case TK_HEX:
          case TK_REG:
            tokens[nr_token_local].type = rules[i].token_type;
            memcpy(tokens[nr_token_local].str, substr_start, substr_len);
            break;
          default: 
            tokens[nr_token_local].type = rules[i].token_type;
            break;
        }
        nr_token_local++;
        break;
      }
    }
    
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  nr_token = nr_token_local;
  return true;
}
static bool check_parentheses(int p, int q) {
    // Step 1: 检查整个子表达式是否被一对括号包围
    if (tokens[p].type != '(' || tokens[q].type != ')') {
        return false;
    }

    // Step 2: 使用计数器来检查括号匹配情况
    int stack = 0;  // 用一个整数来模拟栈的深度

    for (int i = p; i < q; i++) {  // 遍历从 p 到 q 范围内的字符
        if (tokens[i].type == '(') {
            stack++;  // 遇到左括号，栈加一
        } else if (tokens[i].type == ')') {
            stack--;  // 遇到右括号，栈减一
            if (stack <= 0) {
                return false;  // 如果栈深度为负，说明右括号多于左括号
            }
        }
    }


    // Step 3: 如果栈深度为零，说明所有的括号都匹配
    return stack == 1;
}
static word_t eval(int p, int q) {
  printf("p is %d, q is %d\n", p, q);
  if (p > q) {
    /* Bad expression */
    return false;
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    if (tokens[p].type == TK_NUM)
    {
      return atoi(tokens[p].str);
    }else if (tokens[p].type == TK_HEX)
    {
      return (uint32_t)strtoul(tokens[p].str, NULL, 16);
    }else if (tokens[p].type == TK_REG)
    { 
      bool success = false;
      word_t val = isa_reg_str2val(tokens[p].str, &success); 
      if (success == true)
      {
        return val;
      }else
      {
        return false;
      }
      
           
    }
    return false;
      
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  }else if (tokens[p].type == TK_NOTYPE)
  {
    return eval(p+1,q);
  }else if (tokens[q].type == TK_NOTYPE)
  {
    return eval(p,q-1);
  }else {
    // op = the position of 主运算符 in the token expression;
    int op = -1;
    int min_priority = 100;
    int cur_priority = 50;
    int braket_depth = 0;
    for(int i = p; i <= q; i++){
      switch (tokens[i].type)
      {
      case '+':
        cur_priority = 1;
        break;
      case '-':
        cur_priority = 1;
        break;      
      case '*':
        cur_priority = 2;
        break;
      case '/':
        cur_priority = 2;
        break;
      case 'TK_DEREF':
        cur_priority = 3;
        break;
      default:
        break;
      }

      if(tokens[i].type == '('){
        braket_depth ++;
      }else if(tokens[i].type == ')'){
        braket_depth --;
      }

      if (braket_depth == 0 && (tokens[i].type == '+' || tokens[i].type == '-' || tokens[i].type == TK_DEREF || tokens[i].type == '*' || tokens[i].type == '/'))
      { 
        // printf("i is %d, braket_depth is %d\n", i, braket_depth);
        if(cur_priority < min_priority){
          min_priority = cur_priority;
          op = i;
          // printf("op is %d\n", op);
        }
        else if(cur_priority == min_priority)
        {
          op = i;
          // printf("op is %d\n", op);
        }
      }
            
    }
    
    int val1,val2 = 0;

    if (tokens[op].type == TK_DEREF)
    {
      if(op == 0){
        val2 = eval(op + 1, q);
        return vaddr_read(val2, 4);
      }else{
        val1 = eval(p, op - 2);
        int adress_val = eval(op + 1, q);
        val2 = vaddr_read(adress_val, 4);
        op = op-1;
      }
    }else
    {
      val1 = eval(p, op - 1);
      // printf("val1 is %d\n", val1);
      val2 = eval(op + 1, q);
      // printf("val2 is %d\n", val2);
    }
    
    
    

    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      default: assert(0);
    }
  }
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  /* TODO: Insert codes to evaluate the expression. */
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '*' && (i == 0 || tokens[i - 1].type == '+' || tokens[i - 1].type == '-'|| tokens[i - 1].type == '*' || tokens[i - 1].type == '/' )) {
        tokens[i].type = TK_DEREF;
      }
  }
  printf("nr_token is %d\n", nr_token);
  for (size_t i = 0; i < nr_token; i++)
  {
    printf("tokens[%ld].type is %d\n", i, tokens[i].type);
    if (tokens[i].type == TK_NUM||tokens[i].type == TK_HEX)
    {
      printf("tokens[%ld].str is %s\n", i, tokens[i].str);
    }
    
  }
  *success = true;
  //nr_token -1 p 3+*(0x80000000) p *(0x80000000)
  return eval(0, nr_token -1);
}
