#include <stdio.h>

// 공백 spaces개, 별 stars개를 한 줄에 출력하는 함수
void printRow(int spaces, int stars) {
    int i;
    for (i = 0; i < spaces; i++) printf(" ");   // 앞쪽 공백
    for (i = 0; i < stars;  i++) printf("*");   // 별
    printf("\n");
}

// 길이 n, 종류 type 에 맞는 별 모양을 출력하는 함수
void printStar(int n, int type) {
    int mid = (n + 1) / 2;   // 가운데 줄 번호(= 최대 별 개수)
    int r;
    for (r = 1; r <= n; r++) {
        // k1: 1,2,...,mid,...,2,1  (가운데로 갈수록 커짐)
        int k1 = (r <= mid) ? r : (n - r + 1);
        // k2: mid,...,1,...,mid  (가운데로 갈수록 작아짐)
        int k2 = (r <= mid) ? (mid - r + 1) : (r - mid + 1);

        if      (type == 1) printRow(0,        k1);          // 왼쪽 정렬
        else if (type == 2) printRow(mid - k1, k1);          // 오른쪽 정렬
        else if (type == 3) printRow(mid - k2, 2 * k2 - 1);  // 가운데 정렬(모래시계)
        else if (type == 4) printRow(mid - k2, k2);          // 오른쪽 정렬(뒤집힌 모양)
    }
}

int main(void) {
    int n, type;
    printf("사이즈와 종류를 입력하시오.");
    scanf("%d %d", &n, &type);

    // ---- 예외 처리 ----
    if (n % 2 == 0) {                  // 길이가 짝수이면 지원하지 않음
        printf("길이는 홀수만 지원합니다.\n");
        return 0;
    }
    if (type < 1 || type > 4) {        // 종류는 1~4만 지원
        printf("종류는 1~4만 지원합니다.\n");
        return 0;
    }

    printStar(n, type);
    return 0;
}