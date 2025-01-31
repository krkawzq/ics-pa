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
int wp_no = 1;

void init_wp_pool() {
  wp_no = 1;
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

  // 从head链表中删除wp节点
  if (head == NULL) {
    Assert(false, "head is NULL");
  }

  if (head == wp) { // wp是头节点
    head = wp->next;
  } else { // wp不是头节点,需要找到wp的前驱节点
    WP *prev = head;
    while (prev->next != wp) {
      prev = prev->next;
      if (prev == NULL) { // 遍历完整个链表都没找到wp
        Assert(false, "wp not found in head list");
      }
    }
    prev->next = wp->next;
  }

  // 将wp加入free_链表
  wp->next = free_;
  free_ = wp;

  wp_used_count--;
  Assert(wp_used_count >= 0, "wp_used_count < 0");
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
  bool success = true;
  while (wp != NULL) {
    value = expr(wp->expr, &success);
    if (!success) Assert(false, "unknown error in check_watchpoint");
    if (wp->value != value) {
      printf("watchpoint %d: %s\n", wp->NO, wp->expr);
      printf("old value: %u\n", wp->value);
      printf("new value: %u\n", value);
      wp->value = value;
      changed = true;
    }
    wp = wp->next;
  }
  return changed;
}

void delete_watchpoint(int n) { // n 是head中的编号
  WP *wp = head;
  while (wp != NULL) {
    if (wp->NO == n) {
      free_wp(wp);
      break;
    }
    wp = wp->next;
  }
  if (wp == NULL) {
    printf("watchpoint %d deleted\n", n);
  }
}


