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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  /* TODO: Add more members if necessary */
  char *expr;
  word_t value;
  bool is_active;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

static void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].expr = NULL;
    wp_pool[i].value = 0;
    wp_pool[i].is_active = false;
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

static WP* new_wp(){
  //没有空闲监视点结构
  assert(free_ != NULL);
  WP *wp = free_;
  free_ = free_->next;
  wp->next = head;
  head = wp;
  wp->is_active = true;
  return wp;
}

static void free_wp(WP *wp){

  assert(wp != NULL && wp->is_active);

  if(wp == head){
    head = wp->next;
  }else{
    WP *prev = head;
    while (prev->next != wp)
    {
      prev = prev->next;
    }
    prev->next = wp->next;
  }
  wp->next = free_;
  free_ = wp;
  wp->is_active = false;
  wp->expr = NULL;
  wp->value = 0;
}