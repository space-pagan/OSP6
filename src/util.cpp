/* Author:      Zoya Samsonov
 * Created:     November 17, 2020
 * Last Edit:   November 17, 2020
 */

#include "util.h"

const int* range::begin() {
    return ptr;
}

const int* range::end() {
    return &(this->data[this->size]);
}
