#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <cmath>
#include <iostream>
#include <vector>
#include "error_handler.h"

struct range {
    int* data;
    int* ptr;
    int size;
    range(int T);
    range(int S, int T);
    range(int S, int T, int U);
    const int* begin();
    const int* end();
};

template <typename T>
struct ListNode {
    T val;
    bool init = false;
    ListNode *prev, *next;

    ListNode(T v, ListNode<T>* p, ListNode<T>* n) : 
        val(v),
        init(true),
        prev(p),
        next(n)
    {}
    
    ListNode() : ListNode(NULL, NULL, NULL) {
        this->init = false;
    }

    ListNode(T v) : ListNode(v, NULL, NULL) {}

    ListNode<T>* front() {
        if (this->prev) return this->prev->front();
        return this;
    }

    ListNode<T>* back() {
        if (this->next) return this->next->back();
        return this;
    }

    void remove_ref() {
        if (this->prev) this->prev->next = this->next;
        if (this->next) this->next->prev = this->prev;
    }

    void insert_before(ListNode<T>* src) {
        if (this->prev) this->prev->next = src;
        src->prev = this->prev;
        src->next = this;
        this->prev = src;
    }

    void move(ListNode<T>* dest) {
        if (this != dest) {
            this->remove_ref();
            dest->insert_before(this);
        }
    }

    ListNode<T>* erase() {
        this->remove_ref();
        ListNode<T>* f = this->front();
        delete this;
        return f;
    }

    void push_back(T v) {
        if (this->next) this->next->push_back(v);
        else this->next = new ListNode(v, this, NULL);
    }

    void push(T v) {
        if (this->prev) this->prev->push(v);
        else this->prev = new ListNode(v, NULL, this);
    }
};

template <typename T>
struct list {
    ListNode<T>* f = NULL;

    list() {}

    ListNode<T>* front() {
        return f;
    }

    ListNode<T>* back() {
        if (f) return f->back();
        return NULL;
    }

    void move(ListNode<T>* dest, ListNode<T>* src) {
        if (src && dest) {
            src->move(dest);
            f = src->front();
        }
    }

    void insert(ListNode<T>* dest, ListNode<T>* src) {
        if (src && dest) {
            dest->insert_before(src);
        }
    }

    void insert(ListNode<T>* dest, T v) {
        if (dest) {
            dest->insert_before(new ListNode<T>(v));
        }
    }

    void erase(ListNode<T>* target) {
        ListNode<T>* ptr = f;
        while (ptr && ptr != target) ptr = ptr->next;
        if (ptr) f = ptr->erase();
    }

    ListNode<T>* find(T v) {
        ListNode<T>* ptr = f;
        while (ptr && ptr->val != v) ptr = ptr->next;
        return ptr;
    }

    void push_back(T v) {
        if (f && f->init) f->push_back(v);
        else f = new ListNode<T>(v);
    }

    void push(T v) {
        if (f && f->init) f->push(v);
        else f = new ListNode<T>(v);
        f = f->front();
    }
};

struct roullette {
    int size;
    std::vector<float> sum;

    roullette(int s);
    int rand();
};

std::string epochstr(); // gets the unix epoch as a string
long floattimetons(float time); // convert s.xxxxxx to ns

#endif
