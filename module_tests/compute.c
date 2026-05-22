#include "compute.h"

int Compute1(int a, int b) {
    int sum = 0;
    for (int i = 0; i < a; i++) {
        sum = sum + b;
    }

    return sum;
}

int Compute2(int a, int b) {
    int sum = 0;
    for (int i = 0; i < a; i++) {
        sum = sum + Compute1(a, b);
    }

    return sum;
}