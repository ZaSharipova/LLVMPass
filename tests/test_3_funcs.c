#include <stdio.h>

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

int main(void) {
    int n = 7, m = 4;
    int result = Compute2(m, n);
    printf("%d\n", result);
    
    return 0;
}