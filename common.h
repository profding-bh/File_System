//
//  common.h
//  analy_inode
//
//  Created by 丁贵强 on 16/4/15.
//  Copyright © 2016年 丁贵强. All rights reserved.
//

#ifndef common_h
#define common_h



struct list_head {
    struct list_head *next, *prev;
};

typedef struct {
    volatile int counter;
} atomic_t;

struct hlist_node {
    struct hlist_node *next, **pprev;
};

struct hlist_head {
    struct hlist_node *first;
};


typedef unsigned long long u64;
typedef unsigned short umode_t;

#ifdef __GNUC__
typedef long long       __kernel_loff_t;
#endif

#if defined(__GNUC__)
typedef __kernel_loff_t         loff_t;
#endif

typedef struct { } arch_spinlock_t;

typedef struct raw_spinlock {
    arch_spinlock_t raw_lock;
} raw_spinlock_t;

typedef struct spinlock {
    struct raw_spinlock rlock;
} spinlock_t;



struct mutex {
    /* 1: unlocked, 0: locked, negative: locked, possible waiters */
    atomic_t                count;
    spinlock_t              wait_lock;
    struct list_head        wait_list;
#ifdef CONFIG_DEBUG_MUTEXES
    struct thread_info      *owner;
    const char              *name;
    void                    *magic;
#endif
#ifdef CONFIG_DEBUG_LOCK_ALLOC
    struct lockdep_map      dep_map;
#endif
};



#endif /* common_h */
