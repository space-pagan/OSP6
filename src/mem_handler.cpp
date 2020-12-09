/*  Author:     Zoya Samsonov
 *  Created:    December  2, 2020
 *  Last edit:  December  2, 2020
 */

#include "error_handler.h"
#include "util.h"
#include "mem_handler.h"

memman::memman() {
    for (int i : range(256)) {
        lru_order.push(i);
        frames.emplace_back(i, lru_order.front());
    }
}

int memman::in_frame(int proc, int page, bool ignore_waiting = false) {
    for (auto i = 0UL; i < this->frames.size(); i++)
        // only return the frame if the proc and page match, AND
        // either waiting_io is false or ignore_waiting is true
        if (this->frames[i].page == page && 
                this->frames[i].proc == proc && 
                (!this->frames[i].waiting_io || ignore_waiting))
            return i;
    return -1;
}

int memman::check_frame(int proc, int page, int& frameout) {
    frameout = in_frame(proc, page, true);
    int out = 1;
    
    if (frameout == -1) { // page is not loaded
        out = 0;
        frameout = in_frame(-1, -1); // do not ignore waiting_io flag
        if (frameout == -1) { // no empty frames available
            frameout = this->lru_order.back()->val; // run LRU
        }
        // page is not loaded so frame will be waiting for IO
        // this ensures that future check_frame() calls will not pick
        // this frame
        this->frames[frameout].waiting_io = true;
    } 
    // update frame access order
    this->lru_order.move(
            lru_order.front(), this->frames[frameout].last_used_ref);
    return out;
}

void memman::set_dirty(int framenum, int rw) {
    this->frames[framenum].dirty |= rw;
}

int memman::is_dirty(int framenum) {
    return this->frames[framenum].dirty;
}

void memman::load_frame(int framenum, int proc, int page, int rw) {
    this->frames[framenum].proc = proc;
    this->frames[framenum].page = page;
    this->set_dirty(framenum, rw);
    this->frames[framenum].waiting_io = false;
}

void memman::flush_frame(frame& f) {
    f.proc = -1;
    f.page = -1;
    f.dirty = false;
    f.waiting_io = false;
    this->lru_order.move(this->lru_order.front(), f.last_used_ref);
}

void memman::flush_frame(int framenum) {
    this->flush_frame(this->frames[framenum]);
}

void memman::flush_all(int pid) {
    for (frame& f : this->frames) {
        if (f.proc == pid) this->flush_frame(f);
    }
}

void memman::log_mmap(clk* shclk, Log& log, int cur_count, int max_count) {
    char buf[256];
    char* ptr = buf;
    memset(buf, '\0', 256); // just to be safe

    // Header
    ptr += sprintf(ptr, "    ");
    for (int i : range(16)) {
        ptr += sprintf(ptr, "%x ", i);
    }
    ptr += sprintf(ptr, "    KEY");
    log.logline(buf);
    memset(buf, '\0', ptr-buf+1);
    ptr = buf;

    // Body
    for (int row : range(16)) {
        ptr += sprintf(ptr, "0x%x ", row);
        for (int col : range(16)) {
            frame f = this->frames[16*row + col];
            if (f.proc == -1 && f.page == -1) {
                ptr += sprintf(ptr, f.waiting_io ? "+ " : ". ");
            } else {
                if (f.dirty) ptr += sprintf(ptr, f.waiting_io ? "$ " : "! ");
                else ptr += sprintf(ptr, f.waiting_io ? "& " : "* ");
            }
        }
        if (row == 0) {
            ptr += sprintf(ptr, 
                "    .    empty");
        } else if (row == 1) {
            ptr += sprintf(ptr,
                "    +    empty,  ");
            ptr += sprintf(ptr,
                 "loading from disk");
        } else if (row == 2) {
            ptr += sprintf(ptr,
                "    *    loaded, ");
            ptr += sprintf(ptr,
                 "clean");
        } else if (row == 3) {
            ptr += sprintf(ptr,
                "    &    loaded, clean, loading new frame from disk");
        } else if (row == 4) {
            ptr += sprintf(ptr,
                "    !    loaded, dirty");
        } else if (row == 5) {
            ptr += sprintf(ptr,
                "    $    loaded, dirty, swapping new frame from disk");
        } else if (row == 7) {
            ptr += sprintf(ptr,
                "    t = %s", shclk->tostring().c_str());
        } else if (row == 8) {
            ptr += sprintf(ptr,
                "    Current Processes: %2d", cur_count);
        } else if (row == 9) {
            ptr += sprintf(ptr,
                "    Total Processes:  %3d", max_count);
        }
        log.logline(buf);
        memset(buf, '\0', ptr-buf+1);
        ptr = buf;
    }
}
