#include <stdio.h>

// n이 제곱ㄴㄴ수이면 1, 아니면 0
// (1보다 큰 어떤 제곱수로도 나누어떨어지지 않으면 제곱ㄴㄴ수)
int isSquareFree(int n) {
    int d;
    for (d = 2; d * d <= n; d++) {        // 제곱수 4, 9, 16, ...
        if (n % (d * d) == 0) return 0;   // 제곱수로 나누어떨어지면 아님
    }
    return 1;
}

int main(void) {
    int min, max, i, count = 0;

    printf("min :");  scanf("%d", &min);
    printf("max :");  scanf("%d", &max);

    for (i = min; i <= max; i++)          // 개수 세기
        if (isSquareFree(i)) count++;
    printf("제곱 ㄴㄴ수 : %d개\n", count);

    for (i = min; i <= max; i++)          // 실제 수 출력
        if (isSquareFree(i)) printf("%d ", i);
    printf("\n");
    return 0;
}