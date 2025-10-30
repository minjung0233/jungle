#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <termios.h>
#include <sys/ioctl.h>

#define MAX_PATH_LEN 512
#define MAX_FILES 100
#define MAX_NAME_LEN 256

// 색상 코드
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"

typedef struct {
    char name[MAX_NAME_LEN];
    int is_dir;
} FileEntry;

FileEntry files[MAX_FILES];
int file_count = 0;

// 파일 비교 함수 (정렬용)
int compare_files(const void *a, const void *b) {
    FileEntry *f1 = (FileEntry *)a;
    FileEntry *f2 = (FileEntry *)b;
    
    // .. 은 항상 맨 위
    if (strcmp(f1->name, "..") == 0) return -1;
    if (strcmp(f2->name, "..") == 0) return 1;
    
    // 디렉토리를 파일보다 먼저
    if (f1->is_dir && !f2->is_dir) return -1;
    if (!f1->is_dir && f2->is_dir) return 1;
    
    // 같은 타입이면 이름순 정렬
    return strcasecmp(f1->name, f2->name);
}

// 콘솔 화면 지우기
void clear_screen() {
    printf("\033[2J\033[H");
    fflush(stdout);
}

// 키 입력 받기 (non-blocking)
int getch_linux() {
    struct termios oldt, newt;
    int ch;
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    
    return ch;
}

// 현재 디렉토리의 파일/폴더 목록 가져오기
int get_files(const char* path) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char full_path[MAX_PATH_LEN];
    
    file_count = 0;
    
    dir = opendir(path);
    if (dir == NULL) {
        return 0;
    }
    
    while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
        // . 은 제외
        if (strcmp(entry->d_name, ".") == 0) {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        if (stat(full_path, &statbuf) == 0) {
            int is_dir = S_ISDIR(statbuf.st_mode);
            
            // 디렉토리이거나 .c 파일만 표시
            if (is_dir || strstr(entry->d_name, ".c") != NULL) {
                strncpy(files[file_count].name, entry->d_name, MAX_NAME_LEN - 1);
                files[file_count].is_dir = is_dir;
                file_count++;
            }
        }
    }
    
    closedir(dir);
    
    // 정렬: 디렉토리 먼저, 그 다음 이름순
    qsort(files, file_count, sizeof(FileEntry), compare_files);
    
    return file_count;
}

// 메뉴 출력
void print_menu(const char* current_path, int selected) {
    clear_screen();
    printf(CYAN "═══════════════════════════════════════════════════════\n" RESET);
    printf(CYAN "           C 파일 네비게이터 & 디버거\n" RESET);
    printf(CYAN "═══════════════════════════════════════════════════════\n" RESET);
    printf(YELLOW "현재 경로: %s\n" RESET, current_path);
    printf(CYAN "───────────────────────────────────────────────────────\n" RESET);
    
    for (int i = 0; i < file_count; i++) {
        if (i == selected) {
            printf(GREEN "▶ " RESET);
        } else {
            printf("  ");
        }
        
        if (files[i].is_dir) {
            printf(BLUE "📁 %s\n" RESET, files[i].name);
        } else if (strstr(files[i].name, ".c") != NULL) {
            printf(MAGENTA "📄 %s\n" RESET, files[i].name);
        } else {
            printf("   %s\n", files[i].name);
        }
    }
    
    printf(CYAN "───────────────────────────────────────────────────────\n" RESET);
    printf("↑↓: 이동 | Enter: 선택 | ");
    
    if (selected >= 0 && selected < file_count && !files[selected].is_dir && strstr(files[selected].name, ".c")) {
        printf("c: 컴파일 | r: 실행 | d: 디버그 | ");
    }
    
    printf("q: 종료\n");
}

// 파일 컴파일
int compile_file(const char* path, const char* filename, int debug_mode) {
    char cmd[MAX_PATH_LEN * 2];
    char output_name[MAX_NAME_LEN];
    
    // 확장자 제거
    strncpy(output_name, filename, sizeof(output_name) - 1);
    char* ext = strrchr(output_name, '.');
    if (ext) *ext = '\0';
    
    if (debug_mode) {
        snprintf(cmd, sizeof(cmd), "cd '%s' && gcc -g -Wall '%s' -o '%s'", 
                 path, filename, output_name);
        printf(YELLOW "\n컴파일 중 (디버그 모드)...\n" RESET);
    } else {
        snprintf(cmd, sizeof(cmd), "cd '%s' && gcc -Wall '%s' -o '%s'", 
                 path, filename, output_name);
        printf(YELLOW "\n컴파일 중...\n" RESET);
    }
    
    int result = system(cmd);
    
    if (result == 0) {
        printf(GREEN "✓ 컴파일 성공!\n" RESET);
        return 1;
    } else {
        printf(RED "✗ 컴파일 실패!\n" RESET);
        return 0;
    }
}

// 프로그램 실행
void run_program(const char* path, const char* filename) {
    char cmd[MAX_PATH_LEN * 2];
    char output_name[MAX_NAME_LEN];
    
    strncpy(output_name, filename, sizeof(output_name) - 1);
    char* ext = strrchr(output_name, '.');
    if (ext) *ext = '\0';
    
    snprintf(cmd, sizeof(cmd), "cd '%s' && './%s'", path, output_name);
    
    printf(GREEN "\n프로그램 실행 중...\n" RESET);
    printf(CYAN "═══════════════════════════════════════════════════════\n" RESET);
    system(cmd);
    printf(CYAN "═══════════════════════════════════════════════════════\n" RESET);
    printf(YELLOW "\n아무 키나 누르면 계속...\n" RESET);
    getch_linux();
}

// GDB 디버거 실행
void debug_program(const char* path, const char* filename) {
    char cmd[MAX_PATH_LEN * 2];
    char output_name[MAX_NAME_LEN];
    
    strncpy(output_name, filename, sizeof(output_name) - 1);
    char* ext = strrchr(output_name, '.');
    if (ext) *ext = '\0';
    
    printf(GREEN "\nGDB 디버거 시작...\n" RESET);
    printf(CYAN "═══════════════════════════════════════════════════════\n" RESET);
    printf(YELLOW "유용한 GDB 명령어:\n" RESET);
    printf("  run (r)         - 프로그램 실행\n");
    printf("  break main (b)  - main에 브레이크포인트 설정\n");
    printf("  next (n)        - 다음 줄로\n");
    printf("  step (s)        - 함수 안으로 들어가기\n");
    printf("  print <변수> (p)- 변수 값 출력\n");
    printf("  continue (c)    - 계속 실행\n");
    printf("  quit (q)        - 종료\n");
    printf(CYAN "═══════════════════════════════════════════════════════\n" RESET);
    
    snprintf(cmd, sizeof(cmd), "cd '%s' && gdb '%s'", path, output_name);
    system(cmd);
    
    printf(YELLOW "\n아무 키나 누르면 계속...\n" RESET);
    getch_linux();
}

// 메인 네비게이션
void navigate(char* current_path) {
    int selected = 0;
    int key;
    
    while (1) {
        get_files(current_path);
        
        if (file_count == 0) {
            printf(RED "폴더가 비어있습니다!\n" RESET);
            printf(YELLOW "아무 키나 누르면 뒤로...\n" RESET);
            getch_linux();
            return;
        }
        
        if (selected >= file_count) selected = file_count - 1;
        if (selected < 0) selected = 0;
        
        print_menu(current_path, selected);
        
        key = getch_linux();
        
        if (key == 27) {  // ESC 또는 화살표 키
            key = getch_linux();
            if (key == '[') {
                key = getch_linux();
                if (key == 'A') {  // 위
                    selected--;
                    if (selected < 0) selected = file_count - 1;
                } else if (key == 'B') {  // 아래
                    selected++;
                    if (selected >= file_count) selected = 0;
                }
            }
        } else if (key == 10 || key == 13) {  // Enter
            if (files[selected].is_dir) {
                // 디렉토리로 이동
                if (strcmp(files[selected].name, "..") == 0) {
                    // 상위 디렉토리로
                    char* last_slash = strrchr(current_path, '/');
                    if (last_slash && last_slash != current_path) {
                        *last_slash = '\0';
                    }
                } else {
                    // 하위 디렉토리로
                    char new_path[MAX_PATH_LEN];
                    snprintf(new_path, sizeof(new_path), "%s/%s", current_path, files[selected].name);
                    strncpy(current_path, new_path, MAX_PATH_LEN - 1);
                }
                selected = 0;
            }
        } else if (key == 'c' || key == 'C') {  // 컴파일만
            if (!files[selected].is_dir && strstr(files[selected].name, ".c")) {
                compile_file(current_path, files[selected].name, 0);
                printf(YELLOW "\n아무 키나 누르면 계속...\n" RESET);
                getch_linux();
            }
        } else if (key == 'r' || key == 'R') {  // 컴파일 후 실행
            if (!files[selected].is_dir && strstr(files[selected].name, ".c")) {
                if (compile_file(current_path, files[selected].name, 0)) {
                    run_program(current_path, files[selected].name);
                } else {
                    printf(YELLOW "\n아무 키나 누르면 계속...\n" RESET);
                    getch_linux();
                }
            }
        } else if (key == 'd' || key == 'D') {  // 디버그
            if (!files[selected].is_dir && strstr(files[selected].name, ".c")) {
                if (compile_file(current_path, files[selected].name, 1)) {
                    debug_program(current_path, files[selected].name);
                } else {
                    printf(YELLOW "\n아무 키나 누르면 계속...\n" RESET);
                    getch_linux();
                }
            }
        } else if (key == 'q' || key == 'Q') {  // 종료
            return;
        }
    }
}

int main() {
    char current_path[MAX_PATH_LEN];
    
    // 현재 디렉토리 가져오기
    if (getcwd(current_path, sizeof(current_path)) == NULL) {
        printf(RED "현재 디렉토리를 가져올 수 없습니다!\n" RESET);
        return 1;
    }
    
    printf(GREEN "C 파일 네비게이터를 시작합니다...\n" RESET);
    printf("현재 위치: %s\n", current_path);
    sleep(1);
    
    navigate(current_path);
    
    clear_screen();
    printf(GREEN "프로그램을 종료합니다. 안녕히 가세요!\n" RESET);
    
    return 0;
}
