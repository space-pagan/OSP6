#include <iostream>
#include <vector>
#include <cstring>
#include "util.h"

struct frame {
    int proc;
    int page;
    bool dirty;
    ListNode<int>* last_used_ref;

    frame(int pr, int pg, ListNode<int>* lur) : 
        proc(pr),
        page(pg),
        last_used_ref{lur}
    {
        
    }
};

int inframe(std::vector<frame> frames, int proc, int page) {
    for (auto i = 0UL; i < frames.size(); i++) {
        if (frames[i].page == page && frames[i].proc == proc) return i;
    }
    return -1;
}

int main(int argc, char** argv) {
    std::vector<frame> frames;
    list<int> l;
    roullette r(32);

    for (int i = 0; i < 256; i++) {
        l.push(i);
        frames.emplace_back(-1, -1, l.front());
    }

    for (int i = 0; i < 1024; i++) {
        int proc = rand() % 18;
        int page = r.rand();
        int frameref;
        bool setdirty = rand() % 4 == 0;
        printf("P%-2d pg%-2d\t", proc, page);
        if ((frameref = inframe(frames, proc, page)) != -1) {
            std::cout << "found in frame " << frameref << "\n";
            l.move(l.front(), frames[frameref].last_used_ref);
        } else {
            frameref = l.back()->val;
            std::cout << "not in frame\tLoaded into frame " << frameref << "\n";
            l.move(l.front(), l.back()); // move last element (LRU) to front (MRU)
            frames[frameref].proc = proc;
            frames[frameref].page = page;
        }
        if (!frames[frameref].dirty) frames[frameref].dirty = setdirty;

        if (i % 16 == 15) {
            int j = 0;
            int k = 0;
            printf("\n    ");
            for (int i : range(16)) printf("%x ", i);
            printf("\n0x%x ", k);
            for (frame f : frames) {
                if (f.proc == -1 || f.page == -1) printf(". ");
                else if (f.dirty) printf("! ");
                else printf("+ ");
                j++;
                if (j >= 16) {
                    j = 0; 
                    std::cout << "\n";
                    k++;
                    if (k < 16) printf("0x%x ", k);
                    else printf("\n");
                }
            }
        }
    }
}
