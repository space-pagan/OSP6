/*  Author:     Zoya Samsonov
 *  Created:    December  2, 2020
 *  Last edit:  December  2, 2020
 */

#include "error_handler.h"
#include "util.h"
#include "mem_handler.h"

int memman::in_frame(int proc, int page) {
    for (auto i = 0UL; i < this->frames.size(); i++)
        if (this->frames[i].page == page && this->frames[i].proc == proc)
            return i;
    return -1;
}

int memman::check_frame(int proc, int page, int& frameout) {
    frameout = in_frame(proc, page);
    int out = 0;
    
    if (frameout == -1) {
        out = -1;
        // check if any empty frames
        frameout = in_frame(-1, -1);
        if (frameout == -1) {
            // no, run LRU
            frameout = this->lru_order.back()->val;
        }
    } 
    this->lru_order.move(lru_order.front(), this->frames[frameout].last_used_ref);
    return out;
}

void memman::load_frame(int framenum, int proc, int page, int rw) {
    this->frames[framenum].proc = proc;
    this->frames[framenum].page = page;
    this->frames[framenum].dirty |= rw;
}

void memman::flush_frame(frame &f) {
    f.proc = -1;
    f.page = -1;
    f.dirty = -1;
    this->lru_order.move(this->lru_order.front(), f.last_used_ref);
}

void memman::flush_frame(int framenum) {
    this->flush_frame(this->frames[framenum]);
}

void memman::flush_all(int pid) {
    for (frame f : this->frames) {
        if (f.proc == pid) this->flush_frame(f);
    }
}
