#include "util.h"
#include <iostream>

template <typename T> void printlist(list<T>* l) {
    for (auto ptr = l->front(); ptr; ptr = ptr->next) {
        std::cout << ptr->val << " ";
    }
    std::cout << "\n";
}

int main(int argc, char** argv) {
    list<int> l;
    l.push(3);
    l.push_back(2);
    l.push_back(1);
    l.push(0);
    l.insert(l.find(3), 4);

    printlist(&l);

    l.move(l.front(), l.find(1));

    printlist(&l);

    l.erase(l.find(3));

    printlist(&l);
}
