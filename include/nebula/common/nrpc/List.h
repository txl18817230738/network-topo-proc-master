// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include <stddef.h>

namespace nebula::nrpc {

// clang-format off
struct ListNode {
    ListNode    *prev;
    ListNode    *next;
};

struct List {
    List() {
        root.prev = &root;
        root.next = &root;
    }

    bool empty() const {
        return root.next == &root;
    }

    void clear() {
        root.prev = &root;
        root.next = &root;
    }

    void append(ListNode *node);

    void append(List &rhs);

    void prepend(ListNode *node);

    static void remove(ListNode *node);

    void move(List &rhs);

    ListNode    root;
};


#define __decltype(type) std::remove_reference_t<decltype(type)>
#define __offsetof(type, node) reinterpret_cast<size_t>(&(reinterpret_cast<type*>(0))->node)

#define __list_element(ptr, type, node)                                                     \
    reinterpret_cast<type*>(reinterpret_cast<char*>(ptr) - __offsetof(type, node))

#define NRPC_LIST_FOR_EACH(cur, list, node)                                                      \
    for (cur = __list_element((list).root.next, __decltype(*cur), node);                    \
            &cur->node != &(list).root;                                                     \
            cur = __list_element(cur->node.next, __decltype(*cur), node))

#define NRPC_LIST_FOR_EACH_SAFE(cur, list, node)                                                 \
    __decltype(cur) __next = nullptr;                                                       \
    for (cur = __list_element((list).root.next, __decltype(*cur), node),                    \
            __next = __list_element(cur->node.next, __decltype(*cur), node);                \
            &cur->node != &(list).root;                                                     \
            cur = __next,                                                                   \
            __next = __list_element(__next->node.next, __decltype(*cur), node))

#define NRPC_LIST_HEAD(list, type, node)                                                         \
    ((list).empty() ? nullptr : __list_element((list).root.next, type, node))

#define NRPC_LIST_TAIL(list, type, node)                                                         \
    ((list).empty() ? nullptr : __list_element((list).root.prev, type, node))

}   // namespace nebula::nrpc

