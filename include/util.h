#ifndef UTIL_H
#define UTIL_H

struct range {
    int* data;
    int* ptr;
    int size;
    const int* begin();
    const int* end();
    range(int T) {
        data = new int[T];
        size = T;
        int i = 0;
        while (i < T) {
            data[i] = i;
            i++;
        }
        ptr = data;
    }
};

#endif
