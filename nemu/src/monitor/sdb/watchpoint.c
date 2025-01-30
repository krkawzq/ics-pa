/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
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



static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;
int wp_used_count = 0;

void init_wp_pool() {
  wp_used_count = 0;
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

WP* new_wp() {
  if (free_ == NULL) {
    Assert(false, "no free watchpoint");
  }
  WP *wp = free_;
  free_ = free_->next;
  if (head == NULL) {
    head = wp;
    wp->next = NULL;
  } else {
    wp->next = head;
    head = wp;
  }
  wp_used_count++;
  return wp;
}

void free_wp(WP *wp) {
  if (wp == NULL) {
    Assert(false, "wp is NULL");
  }
  // 需要将wp从head链表中删除
  WP *p = head;
  bool found = false;
  while (p != NULL) {
    if (p->next == wp) {
      p->next = wp->next;
      found = true;
      break;
    }
    p = p->next;
  }
  if (!found) {
    Assert(false, "wp not found");
  }
  if (free_ == NULL) {
    free_ = wp;
    free_->next = NULL;
  } else {
    wp->next = free_;
    free_ = wp;
  }
  wp_used_count--;
}

static void _print_watchpoint(WP *wp) { // 递归后序遍历
  if (wp == NULL) {
    return;
  }
  _print_watchpoint(wp->next);
  printf("watchpoint %d: %s\n", wp->NO, wp->expr);
}

void print_watchpoint() {
  _print_watchpoint(head);
}

bool check_watchpoint() {
  if (wp_used_count == 0) {
    return false;
  }
  WP *wp = head;
  word_t value;
  bool changed = false;
  while (wp != NULL) {
    value = expr(wp->expr, NULL);
    if (wp->value != value) {
      printf("watchpoint %d: %s\n", wp->NO, wp->expr);
      printf("old value: %d\n", wp->value);
      printf("new value: %d\n", value);
      wp->value = value;
      changed = true;
    }
    wp = wp->next;
  }
  return changed;
}

void delete_watchpoint(int n) {
  WP *wp = head;
  for (int i = 0; i < wp_used_count - n; i++) { // 反向编号的
    wp = wp->next;
  }
  free_wp(wp);
  for (int i = 0; i < wp_used_count; i++) {
    wp_pool[i].NO = wp_used_count - i; // 重新编号, head -> 3, 2, 1
  }
}


