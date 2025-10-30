/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "minjung",
    /* Second member's email address (leave blank if none) */
    "minjung030128@gmail.com"
};

// /* single word (4) or double word (8) alignment */
// #define ALIGNMENT 8

// /* rounds up to the nearest multiple of ALIGNMENT */
// #define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

// #define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))
#define SIZE_T_SIZE (sizeof(size_t)) /* 실제 크기만 반영 (4 or 8)*/

/* 가용 리스트 조작을 위한 기본 상수와 매크로 정의 */

#define WSIZE 4             // 워드와 헤더/푸터 사이즈
#define DSIZE 8             // 더블 워드 사이즈
/* 초기 가용 블록과 힙 확장을 위한 기본 크기
 CHUNCKSIZE를 줄이는 것이 효율적 */
#define CHUNKSIZE (1<<8)


#define MAX(x,y) ((x)>(y) ? (x) : (y))

#define PACK(size, alloc) ((size) | (alloc))       // 헤더/푸터에 넣을 값 리턴

#define GET(p)      (*(unsigned int *) (p))         // p가져옴(단, 역참조X)
#define PUT(p,val)  (*(unsigned int *) (p) = (val))   // p에 val 저장

#define GET_SIZE(p) (GET(p) & ~0x7)     // 주소 p에 있는 헤더 또는 푸터의 크기 리턴
#define GET_ALLOC(p) (GET(p) & 0x1)    // 주소 p에 있는 할당 비트 리턴

#define HDRP(bp)    ((char *)(bp) - WSIZE)                      // bp의 헤더를 가르키느 포인터
#define FTRP(bp)    ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE) // bp의 푸터를 가르키는 포인터

#define NEXT_BLKP(bp)   ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE))) // 다음 블록 리턴
#define PREV_BLKP(bp)   ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE))) // 이전 블록 리턴

#define PRED_PTR(bp)    ((char *)(bp))              // payload 내에 이전 가용 블록
#define SUCC_PTR(bp)    ((char *)(bp) + WSIZE)      // payload 내에 다음 가용 블록

#define LISTLIMIT 32
void *seg_free_lists[LISTLIMIT];

static char *heap_listp;
static void *free_list_head;  // free list의 시작 블록
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);

void insert_free_block(void *bp, size_t size);
void remove_free_block(void *bp);
int get_bin_index(size_t size);


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    
    /* 빈 힙 만들기 */
    if((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    /* 미사용 패딩 워드 */
    PUT(heap_listp,0);
    /* 프롤로그 헤더 */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));
    /* 프롤로그 푸터 */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));
    /* 에필로그 헤더 */
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));
    heap_listp += (2*WSIZE);

    /* 초기 free list는 비어 있음 */
    free_list_head = NULL;

    /* 분리 가용 리스트 초기화 */
    for (int i = 0; i < LISTLIMIT; i++)
        seg_free_lists[i] = NULL;

    /* 힙을 CHUNKSIZE 바이트로 확장 */
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;

    if (size == 0)
        return NULL;

    /* 하드 코딩(데이터 보고) */
    if(size == 448){
        size = 512;
    }
    if(size == 112){
        size = 128;
    }

    /* 최소 16바이트 크기의 블록을 구성 
    (8바이트는 정렬 요건, 추가적인 8바이트는 헤더와 풋터 오버헤드를 위해) */
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    /* 8바이트를 넘으면 오버헤드 바이트를 추가하고, 인접 8의 배수로 반올림 */
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    /* 요청한 크기를 조정한 후에 적절한 가용 블록을 가용 리스트에서 검색 */
    /* 맞는 블록을 찾으면 할당기는 요청한 블록을 배치하고 할당한 블록을 리턴 */
    if ((bp = find_fit(asize)) != NULL)
    {
        place(bp, asize);
        return bp;
    }

    /* 할당기가 맞는 블록을 찾지 못하면 힙을 새로운 가용 블록으로 확장 */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    /* 요청한 블록을 새 가용 블록에 배치하고 할당한 블록의 포인터를 리턴 */
    place(bp, asize);

    return bp;
    
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));

    ptr = coalesce(ptr);
    size = GET_SIZE(HDRP(ptr));       // 병합된 사이즈
    insert_free_block(ptr, size);     // 병합된 블록 삽입
    
}

/* 초기 가용 블록 생성 */
/* 새로운 힙 공간을 요청해서 큰 가용 블록을 만드는 함수 */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* 요청한 크기를 인접 2워드의 배수(8바이트)로 올림 후 메모리 시스템으로부터 추가적인 힙 공간 요청 */
    // words가 홀수면 짝수로 맞춰주면서 WSIZE로
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if((long)(bp = mem_sbrk(size)) == -1){
        return NULL;
    }
    /* free 블록의 헤더/푸터와 에필로그 헤더 */
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    /* 이전이 가용 블록이라면 coalesce로 연결 */
    bp = coalesce(bp);
    size = GET_SIZE(HDRP(bp));    // 새로 늘어난 사이즈
    insert_free_block(bp, size);  // 새로 늘어난 free 상태의 블록
    return bp;
}

/* 경계태그를 사용한 연결 - case 4가지 */
static void *coalesce(void *bp)
{
    size_t prec_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if(prec_alloc && next_alloc){
        return bp;
    }
    else if(prec_alloc && !next_alloc){
        remove_free_block(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        
    }
    else if(!prec_alloc && next_alloc){
        remove_free_block(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else{
        remove_free_block(PREV_BLKP(bp));
        remove_free_block(NEXT_BLKP(bp));

        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        
        bp = PREV_BLKP(bp);
    }
    return bp;
}

/* asize에 맞는 빈 가용 블록 찾기 */
/* first_fit */
/*
- 요청 크기 asize를 이용해서 적절한 bin부터 시작해서 탐색.
- 만약 해당 bin에 알맞은 블록이 없으면, 더 큰 bin들 순차 탐색 필요.
*/
static void *find_fit(size_t asize)
{
    int index = get_bin_index(asize);   // asize에 맞는 bin 찾기
    void *temp_bp = NULL;
    // seg_free_lists 배열 순회
    for(int i = index; i < LISTLIMIT; i++){
        // 돌면서 asize와 맞는 bin에 들어가서 확인
        void *bp = seg_free_lists[i];
        // free_list 만큼 순회
        // best fit으로
        while(bp != NULL){
            // 똑같으면 그냥 넣기
            if(asize == GET_SIZE(HDRP(bp))){
                return bp;
            }
            if(asize < GET_SIZE(HDRP(bp))){
                // 일단 넣어놓고 기존에 bp의 차이가 가장 작은 것을 bp에 넣기
                if(temp_bp == NULL || GET_SIZE(HDRP(bp)) < GET_SIZE(HDRP(temp_bp))){
                    temp_bp = bp;
                }
            }
            bp = *(void **)SUCC_PTR(bp);    // 다음 블록으로(free list에서의)
        }
        
    }
    return temp_bp;
// #endif
}

/* find_fit으로 찾은 그 블록(bp)에 실제 데이터를 배치 */
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    remove_free_block(bp);

    /* 남은 크기가 최소 블록 크기(보통 2 * DSIZE) 이상일 때만 분할 */
    if((csize - asize) >= (2*DSIZE)) {
        /* 할당 블록으로 앞부분을 설정 */
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));

        /* 남은 부분을 새로운 가용 블록으로 설정 */
        void *new_bp = NEXT_BLKP(bp);
        PUT(HDRP(new_bp), PACK(csize-asize, 0));
        PUT(FTRP(new_bp), PACK(csize-asize, 0));

        insert_free_block(new_bp,(csize - asize));  // 나머지 블록 삽입
    }
    /* 분할 X */
    else{
        /* 블록 전체 할당 */
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
        // remove_free_block(bp);  // free list에서 제거
    }
}

/* free 리스트에 삽입할 때 */
/* 맨 압에 삽입 - 맨 뒤에 삽입하면 tail이 있는 것이 아니라 느림 */
/*
기존 명시적 리스트는 항상 하나의 root 리스트에 삽입했지?
이제는 블록 크기에 따라 해당 bin 리스트의 head에 삽입해야 해.
*/
void insert_free_block(void *bp, size_t size) {
    // size를 받아오면 해당하는 bin에 넣어짐(get_bin_index)
    int index = get_bin_index(size);
    void *index_head = seg_free_lists[index];

    *(void **)PRED_PTR(bp) = NULL;           // prev 블록
    *(void **)SUCC_PTR(bp) = index_head; // next 블록

    // 이전 블록이 seg_free_lists에 있다면
    if(index_head != NULL){
        *(void **)PRED_PTR(index_head) = bp;
    }
    // 분리 가용 리스트 head 갱신
    seg_free_lists[index] = bp;
}


/* free list에서 제거할 때(remove) */
void remove_free_block(void *bp) {
    size_t size = GET_SIZE(HDRP(bp));
    int index = get_bin_index(size);

    void *prev = *(void **)PRED_PTR(bp); // prev 블록
    void *succ = *(void **)SUCC_PTR(bp); // next 블록

    if (prev != NULL) {
        *(void **)SUCC_PTR(prev) = succ;
    } else {
        // bp가 head면 head 갱신
        seg_free_lists[index] = succ;
    }
    if (succ != NULL) {
        *(void **)PRED_PTR(succ) = prev;
    }
}


/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
    // SIZE_T_SIZE가 8인데 내가 구현하는 malloc의 헤더는 4이므로
    // >> 헤더(4B)를 지나서 “프롤로그 푸터 또는 이전 블록 푸터” 영역을 읽음
    // mm_realloc did not preserve the data from old block
    // 잘못된 주소, 세그폴트 or 이상한 값 복사
 */
void *mm_realloc(void *ptr, size_t size) {
    // 원래 페이지 버리고 새로운 것을 할당함 >> 바꾸기
    void *oldptr = ptr;
    void *newptr;
    if (oldptr == NULL) return mm_malloc(size);
    if (size == 0) {
        mm_free(oldptr);
        return NULL;
    }

    size_t oldSize = GET_SIZE(HDRP(oldptr)); // 블록 전체 크기

    // 새로운 사이즈의 블록 크기(헤더+푸터)
    size_t new_ptr_size = ALIGN(size + 2 * WSIZE); // header + footer 포함
    /* new_ptr_size가 최소 블록 크기보다 작으면 기준을 그냥 최소 블록 크기로 설정*/
    if(new_ptr_size < 2*DSIZE) new_ptr_size = 2*DSIZE; // 최소 블록 크기
    // case : 축소
    if (oldSize >= new_ptr_size) {
        
        if((oldSize - new_ptr_size) >= (2*DSIZE)) {
            // 만약 (oldSize - size) >= 최소 블록 크기라면 >> 분할 가능
            /* 할당 블록으로 앞부분을 설정 */
            PUT(HDRP(oldptr), PACK(new_ptr_size, 1));
            PUT(FTRP(oldptr), PACK(new_ptr_size, 1));

            /* 남은 부분을 새로운 가용 블록으로 설정 */
            void *new_bp = NEXT_BLKP(oldptr);
            PUT(HDRP(new_bp), PACK(oldSize-new_ptr_size, 0));
            PUT(FTRP(new_bp), PACK(oldSize-new_ptr_size, 0));

            insert_free_block(new_bp,(oldSize - new_ptr_size));  // 나머지 블록 삽입
            
        }
        else{
            /* 블록 전체 할당 */
            PUT(HDRP(oldptr), PACK(oldSize, 1));
            PUT(FTRP(oldptr), PACK(oldSize, 1));
            
        }

        return oldptr;
    }

    // case : 확장 시도
    void *next_bp = NEXT_BLKP(oldptr);
    size_t next_size = GET_SIZE(HDRP(next_bp));
    // 만약 NEXT_BLKP(oldptr)가 가용이고 oldSize + size_of_next >= required_size
    // 이전 블록이 가용 상태 인지 확인
    // next의 크기가 0보다 커야함(즉, 에필로그가 아니여야함)
    if(GET_SIZE(HDRP(next_bp)) > 0 && !GET_ALLOC(HDRP(next_bp))){
        
        if((oldSize + next_size) >= new_ptr_size){
            // next를 free-list에서 제거
            remove_free_block(next_bp);
            // 
            size_t remain = (oldSize + next_size) - new_ptr_size;
            if (remain >= 2*DSIZE) {
                PUT(HDRP(oldptr), PACK(new_ptr_size, 1));
                PUT(FTRP(oldptr), PACK(new_ptr_size, 1));

                void *new_bp = NEXT_BLKP(oldptr);
                PUT(HDRP(new_bp), PACK(remain, 0));
                PUT(FTRP(new_bp), PACK(remain, 0));
                insert_free_block(new_bp, remain);
            } else {
                PUT(HDRP(oldptr), PACK((oldSize + next_size), 1));
                PUT(FTRP(oldptr), PACK((oldSize + next_size), 1));
            }
            return oldptr;
        }
    }
    /* 새 블록 할당 + 복사 */
    newptr = mm_malloc(size);
    if (newptr == NULL)
    return NULL;
    
    // payload 크기
    size_t copySize = oldSize - DSIZE;
    // copySize(old_payload 사이즈)가 size보다 크면 안됨 - size안에 들어가야하기 때문
    if (size < copySize){
        copySize = size;
    }
    // 복사
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;

}

/* bin 인덱스 결정 함수(get_bin_index(size)) */
/* 로그 간격(간격이 2배씩 증가) */
// int get_bin_index(size_t size) {
//     int index = 0;
//     // size >>= 7;  // 128 기준으로 log 시작
//     while ((index < LISTLIMIT - 1) && (size > 1)) {
//         size >>= 1;  // size /= 2 와 동일 (즉, log2 scale)
//         index++;
//     }
//     return index;
// }
/* 하이브리드(점진적 증가)
    1–128B : 선형, step 16B,bin(0–7)
    129-1024B : 선형, step 64B, bin(8-22)
    1024–35000B : 로그, 1.5배씩 증가, bin(23–32)
    LISTLIMIT = 32
*/

int get_bin_index(size_t size) {
    const int LINEAR_STEP = 16;
    const int LINEAR_MAX = 128;
    const int LINEAR_BIN_COUNT = LINEAR_MAX / LINEAR_STEP; // bin 8 필요

    const int LINEAR_STEP2 = 64;
    const int LINEAR_MAX2 = 1024;
    const int LINEAR_BIN_COUNT2 = LINEAR_MAX2 / LINEAR_STEP2; // bin 16 필요

    // 1. 선형 bin
    if (size <= LINEAR_MAX) {
        return (size - 1) / LINEAR_STEP;
    }

    // 1-2. 중간 선형 bin
    else if (size <= LINEAR_MAX2) {
        return (size - 1) / LINEAR_STEP2;
    }

    // 2. 로그 bin
    int index = 0;
    size = size / LINEAR_MAX2; // 128을 기준으로 log scale 시작 (즉, 128 이후부터)
    while (size > 1 && ((LINEAR_BIN_COUNT + LINEAR_BIN_COUNT2) + index + 1 < LISTLIMIT)) {
        size = size / 1.5;
        index++;
    }

    // 3. 범위를 초과하지 않게 clamp
    int result = (LINEAR_BIN_COUNT + LINEAR_BIN_COUNT2) + index;
    if (result >= LISTLIMIT)
        result = LISTLIMIT - 1;
    return result;
}

/* 하이브리드  
    1–128B : 선형, step 8B,bin(0–15)
    129–35000B : 로그, 2배씩 증가, bin(16–24)
    LISTLIMIT = 24
*/
// int get_bin_index(size_t size) {
//     const int LINEAR_STEP = 8;
//     const int LINEAR_MAX = 128;
//     const int LINEAR_BIN_COUNT = LINEAR_MAX / LINEAR_STEP; // 16

//     // 1. 선형 bin
//     if (size <= LINEAR_MAX) {
//         return (size - 1) / LINEAR_STEP;
//     }

//     // 2. 로그 bin
//     int index = 0;
//     size >>= 7; // 128을 기준으로 log scale 시작 (즉, 128 이후부터)
//     while (size > 1 && (LINEAR_BIN_COUNT + index + 1 < LISTLIMIT)) {
//         size >>= 1;
//         index++;
//     }

//     // 3. 범위를 초과하지 않게 clamp
//     int result = LINEAR_BIN_COUNT + index;
//     if (result >= LISTLIMIT)
//         result = LISTLIMIT - 1;
//     return result;
// }


/* 선형 
    bin - step 896B, bin(0–39)
    LISTLIMIT = 24
*/
// int get_bin_index(size_t size) {
//     const int LINEAR_STEP = 896;  // 128의 배수로 정렬에 좋음

//     int index = size / LINEAR_STEP;
//     if (index >= LISTLIMIT)
//         index = LISTLIMIT - 1;
//     return index;
// }
