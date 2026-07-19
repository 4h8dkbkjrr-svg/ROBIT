#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main(void) {
    int A[1001];        // 수열 (N은 1000 이하로 가정)
    int N, B, i, j, p = 0, count = 0;

    printf("N : ");  scanf("%d", &N);
    printf("B : ");  scanf("%d", &B);
    for (i = 0; i < N; i++) scanf("%d", &A[i]);

    // B의 위치 찾기 (부분수열은 반드시 이 위치를 포함해야 함)
    for (i = 0; i < N; i++)
        if (A[i] == B) { p = i; break; }

    // 왼쪽 시작점 i 를 p에서 0까지 줄여가며 왼쪽 통계를 누적
    int leftGreater = 0, leftLess = 0;   // 구간 [i..p-1]의 큰수/작은수 개수
    for (i = p; i >= 0; i--) {
        if (i < p) {                     // 새로 포함된 왼쪽 원소 A[i]
            if      (A[i] > B) leftGreater++;
            else if (A[i] < B) leftLess++;
        }
        // 오른쪽 끝점 j 를 p에서 N-1까지 늘려가며 확인
        int greater = leftGreater, less = leftLess;
        for (j = p; j < N; j++) {
            if (j > p) {                 // p는 이미 셈에 반영됨
                if      (A[j] > B) greater++;
                else if (A[j] < B) less++;
            }
            if (greater == less) count++; // 큰수==작은수 → 중앙값 B, 길이 홀수
        }
    }

    printf("중앙값이 %d인 부분수열의 개수 : %d\n", B, count);
    return 0;
}
