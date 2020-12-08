#include <vector>
#include <map>
#include "util.h"

struct request {
    int proc;
    int page;
    int frame;
    int rw;
    long req_time;
    long req_wait;

    request(int proc, int page, int frame, int rw, long time, long wait) :
        proc(proc),
        page(page),
        frame(frame),
        rw(rw),
        req_time(time),
        req_wait(wait)
    {}

    request(const request& old) : 
        proc(old.proc),
        page(old.page),
        frame(old.frame),
        rw(old.rw),
        req_time(old.req_time),
        req_wait(old.req_wait)
    {}
};

struct frame {
    int proc = -1;
    int page = -1;
    bool dirty = false;
    bool waiting_io = false;
    ListNode<int>* last_used_ref;

    frame(ListNode<int>* lur) :
        last_used_ref{lur}
    {}

    frame(int pr, int pg, ListNode<int>* lur) :
        proc(pr), page(pg), last_used_ref{lur}
    {}
};

struct memman {
    std::vector<frame> frames;
    list<int> lru_order;

    memman() {
        for (int i : range(256)) {
            lru_order.push(i);
            frames.emplace_back(lru_order.front());
        }
    }

    int in_frame(int proc, int page, bool ignore_waiting);
    void set_dirty(int framenum, int rw);
    int is_dirty(int framenum);
    void load_frame(int framenum, int proc, int page, int rw);
    int check_frame(int proc, int page, int& frameout);
    void flush_frame(frame &f);
    void flush_frame(int framenum);
    void flush_all(int pid);
};
