#include <stdio.h>

// 두 문자열이 완전히 같으면 1, 다르면 0을 반환 (strcmp 대체)
int isSame(char a[], char b[]) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) return 0;   // 한 글자라도 다르면 다름
        i++;
    }
    return a[i] == b[i];              // 둘 다 끝('\0')에서 만나야 같음
}

int main(void) {
    int inSet[21] = {0};   // inSet[k]=1 이면 k가 집합에 있음 (1~20)
    int seq[20];           // 집합 원소를 "추가된 순서"대로 저장
    int count = 0;         // 현재 원소 개수
    char cmd[20];          // 입력된 명령어를 담는 배열
    int x, k, len;
    char ch;

    printf("연산을 선택하세요. (1 <= x <= 20 )\n");
    printf("add X\nremove X\ncheck X\ntoggle X\nall 0\nempty 0\n\n");

    while (1) {
        printf("input : ");

        // ---- 명령어(알파벳)를 한 글자씩 읽어 cmd에 저장 ----
        len = 0;
        if (scanf(" %c", &ch) != 1) break;   // 입력이 끝나면 종료
        while (ch >= 'a' && ch <= 'z') {
            if (len < 19) cmd[len++] = ch;   // 배열 넘침 방지
            scanf("%c", &ch);
        }
        cmd[len] = '\0';                     // 문자열 끝 표시

        // ---- 명령어 뒤의 숫자 읽기 ----
        if (scanf("%d", &x) != 1) break;

        // ---- 명령어가 "정확히" 일치할 때만 수행 ----
        if (isSame(cmd, "add")) {                     // add x
            if (x < 1 || x > 20) { printf("범위를 벗어났습니다.\n"); continue; }
            if (!inSet[x]) { inSet[x] = 1; seq[count++] = x; }
        }
        else if (isSame(cmd, "remove")) {             // remove x
            if (x < 1 || x > 20) { printf("범위를 벗어났습니다.\n"); continue; }
            if (inSet[x]) {
                inSet[x] = 0;
                for (k = 0; k < count; k++) if (seq[k] == x) break;
                for (; k < count - 1; k++) seq[k] = seq[k + 1]; // 앞으로 당김
                count--;
            }
        }
        else if (isSame(cmd, "check")) {              // check x
            if (x < 1 || x > 20) { printf("범위를 벗어났습니다.\n"); continue; }
            printf("%d ", inSet[x]);   // 있으면 1, 없으면 0 (같은 줄에)
        }
        else if (isSame(cmd, "toggle")) {             // toggle x
            if (x < 1 || x > 20) { printf("범위를 벗어났습니다.\n"); continue; }
            if (inSet[x]) {            // 있으면 제거
                inSet[x] = 0;
                for (k = 0; k < count; k++) if (seq[k] == x) break;
                for (; k < count - 1; k++) seq[k] = seq[k + 1];
                count--;
            } else {                  // 없으면 추가
                inSet[x] = 1; seq[count++] = x;
            }
        }
        else if (isSame(cmd, "all")) {                // all 0
            for (k = 1; k <= 20; k++) inSet[k] = 1;
            for (k = 0; k < 20; k++)  seq[k] = k + 1;
            count = 20;
        }
        else if (isSame(cmd, "empty")) {              // empty 0
            for (k = 1; k <= 20; k++) inSet[k] = 0;
            count = 0;
        }
        else {                                        // 그 외는 잘못된 명령어
            printf("잘못된 명령어입니다.\n");
            continue;
        }

        // ---- 현재 집합 출력 ----
        printf("집합 : { ");
        for (k = 0; k < count; k++) printf("%d, ", seq[k]);
        printf("}\n");
    }
    return 0;
}