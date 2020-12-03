#include <vector>
#include <map>
#include "util.h"

struct frame {
    int proc;
    int page;
    bool dirty;
    ListNode<int>* last_used_ref;

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
            frames.emplace_back(-1, -1, lru_order.front());
        }
    }

    int in_frame(int proc, int page);
    void load_frame(int framenum, int proc, int page, int rw);
    int check_frame(int proc, int page, int& frameout);
    void flush_frame(frame &f);
    void flush_frame(int framenum);
    void flush_all(int pid);
};
