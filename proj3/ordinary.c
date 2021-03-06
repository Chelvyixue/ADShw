#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define SENTINEL -1000000000
/* If the program loops forever, check SENTINEL and set its absolute value larger */
#define INSERT 0
#define BUILDHEAP 1
#define REPEAT 100
#define MAX_MERGE 101

typedef struct ordinary_heap_record *OrdinaryHeap;
struct ordinary_heap_record{
  int capacity;
  int *elements;
  int size;
};

OrdinaryHeap create_ordinary_heap(int capacity);
void insert_ordinary_heap(int element, OrdinaryHeap H);
int delete_min_ordinary_heap(OrdinaryHeap H);
void dispose_ordinary_heap(OrdinaryHeap H);
OrdinaryHeap merge_heap(OrdinaryHeap H1, OrdinaryHeap H2);
OrdinaryHeap merge_ordinary_heap_build(OrdinaryHeap H1, OrdinaryHeap H2);
OrdinaryHeap merge_ordinary_heap_insert(OrdinaryHeap H1, OrdinaryHeap H2);
OrdinaryHeap copy_heap(OrdinaryHeap from, OrdinaryHeap to);

int main(void){
  OrdinaryHeap H[REPEAT][MAX_MERGE];
  FILE *fp;
  int size, each_size, merges;
  int method;
  int x, i, merge_cnt, rpt;
  clock_t start, stop;
  double duration;

  /* initialize */
  fp = fopen("settings.txt", "r");
  fscanf(fp, "%d%d", &size, &merges);
  fclose(fp);
  each_size = size / merges;

  fp = fopen("test_cases/test_all.txt", "r");
  for (merge_cnt = 0; merge_cnt < merges; merge_cnt++){
    H[0][merge_cnt] = create_ordinary_heap(size);
    for (i = 0; i < each_size; i++){
      fscanf(fp, "%d", &x);
      insert_ordinary_heap(x, H[0][merge_cnt]);
    }
  }
  fclose(fp);

  /* copy heap since a merge operation destroy the original heap */
  for (rpt = 1; rpt < REPEAT; rpt++)
    for (merge_cnt = 0; merge_cnt < merges; merge_cnt++)
      H[rpt][merge_cnt] = copy_heap(H[0][merge_cnt], H[rpt][merge_cnt]);

  /* Start test */
  start = clock();

  for (rpt = 0; rpt < REPEAT; rpt++)
    for (merge_cnt = 0; merge_cnt < merges - 1; merge_cnt++)
      H[rpt][merge_cnt + 1] = merge_heap(H[rpt][merge_cnt + 1],
                                         H[rpt][merge_cnt]);

  stop = clock();
  duration = ((double)(stop - start)) / CLOCKS_PER_SEC;

  printf("%lf\n", duration);

  return 0;
}

/* A optimized version of merging on ordinary heaps */
OrdinaryHeap merge_heap(OrdinaryHeap H1, OrdinaryHeap H2){
  double ratio;

  if (H1->size > H2->size)
    ratio = (double)H2->size / H1->size;
  else
    ratio = (double)H1->size / H2->size;

  if (ratio < 0.3)
    return merge_ordinary_heap_insert(H1, H2);
  else
    return merge_ordinary_heap_build(H1, H2);
}

/* Copy heap, used for test since merge operation destroy the original */
OrdinaryHeap copy_heap(OrdinaryHeap from, OrdinaryHeap to){
  to = create_ordinary_heap(from->capacity);
  memcpy(to->elements + 1, from->elements + 1, sizeof(int) * from->size);
  to->size = from->size;

  return to;
}

/* Create an ordinary heap */
OrdinaryHeap create_ordinary_heap(int capacity){
  OrdinaryHeap H;

  H = (OrdinaryHeap)malloc(sizeof(struct ordinary_heap_record));
  if (H == NULL){
    printf("Insufficient space\n");
    exit(1);
  }
  H->capacity = capacity;
  H->elements = (int *)malloc(sizeof(int) * (capacity + 1));
  if (H == NULL){
    printf("Insufficient space\n");
    exit(1);
  }
  H->size = 0;
  H->elements[0] = SENTINEL;

  return H;
}

/* Insert an element into the ordinary heap */
void insert_ordinary_heap(int element, OrdinaryHeap H){
  int i;

  if (H->size == H->capacity){    /* Error: insert to a full heap */
    printf("Insert into a full heap\n");
    exit(1);
  }

  /* percolate up */
  for (i = ++H->size; H->elements[i / 2] > element; i /= 2)
    H->elements[i] = H->elements[i / 2];
  H->elements[i] = element;
}

/* Delete the minimal element in the heap */
int delete_min_ordinary_heap(OrdinaryHeap H){
  int child, i;
  int min, last;

  if (H->size == 0){
    printf("Delete min from an empty heap\n");
    exit(1);
  }

  min = H->elements[1];
  last = H->elements[H->size--];
  for (i = 1; i * 2 < H->size; i = child){
    child = i * 2;
    if (child != H->size && H->elements[child + 1] < H->elements[child])
      child++;
    /* percolate down */
    if (H->elements[child] < last)
      H->elements[i] = H->elements[child];
    else
      break;
  }
  H->elements[i] = last;

  return min;
}

/* Dispose a heap */
void dispose_ordinary_heap(OrdinaryHeap H){
  if (H != NULL){
    free(H->elements);
    free(H);
  }
}

/* Merge two ordinary heaps by insert elements in the heap with
   smaller size into the heap with larger size */
OrdinaryHeap merge_ordinary_heap_insert(OrdinaryHeap H1, OrdinaryHeap H2){
  int i, size;

  if (H1->size < H2->size){
    /* H1 has less elements, insert elements in H1 into H2 */
    size = H1->size;
    for (i = 1; i <= size; i++)
      insert_ordinary_heap(H1->elements[i], H2);
    dispose_ordinary_heap(H1);
    return H2;
  }
  else{
    /* H2 has less elements, insert elements in H2 into H1 */
    size = H2->size;
    for (i = 1; i <= size; i++)
      insert_ordinary_heap(H2->elements[i], H1);
    dispose_ordinary_heap(H2);
    return H1;
  }
}

/* Merge two ordinary heaps by building a new heap */
OrdinaryHeap merge_ordinary_heap_build_old(OrdinaryHeap H1, OrdinaryHeap H2){
  OrdinaryHeap new_heap;
  int build_index, i, child, build_tmp;

  new_heap = create_ordinary_heap(H1->capacity + H2->capacity);
  /* copy the elements into the new heap */
  memcpy(new_heap->elements + 1, H1->elements + 1, sizeof(int) * H1->size);
  memcpy(new_heap->elements + 1 + H1->size, H2->elements + 1, sizeof(int) * H2->size);
  new_heap->size = H1->size + H2->size;
  /* build heap */
  for (build_index = new_heap->size / 2; build_index > 0; build_index--){
    build_tmp = new_heap->elements[build_index];
    /* percolate down */
    for (i = build_index; i * 2 < new_heap->size; i = child){
      child = i * 2;
      if (child != new_heap->size && new_heap->elements[child + 1] < new_heap->elements[child])
        child++;
      if (new_heap->elements[child] < build_tmp)
        new_heap->elements[i] = new_heap->elements[child];
      else
        break;
    }
    new_heap->elements[i] = build_tmp;
  }

  return new_heap;
}

/* Merge two ordinary heaps by building a new heap */
OrdinaryHeap merge_ordinary_heap_build(OrdinaryHeap H1, OrdinaryHeap H2){
  OrdinaryHeap copy_from;
  OrdinaryHeap copy_to;
  /* copy_to->elements.append(copy_from->elements) */
  int build_index, i, child, build_tmp;
  int new_size;

  /* copy the elements into the new heap */
  new_size = H1->size + H2->size;
  if (H1->size < H2->size){
    copy_from = H1;
    copy_to = H2;
  }
  else{
    copy_from = H2;
    copy_to = H1;
  }
  if (copy_to->capacity < new_size){
    printf("Insufficient space while merging two heaps\n");
    exit(1);
  }
  memcpy(copy_to->elements + copy_to->size + 1, copy_from->elements + 1, sizeof(int) * copy_from->size);    /* copy elements from copy_from->elements to copy_to->elements */
  copy_to->size = new_size;
  dispose_ordinary_heap(copy_from);

  /* build heap */
  for (build_index = copy_to->size / 2; build_index > 0; build_index--){
    build_tmp = copy_to->elements[build_index];
    /* percolate down */
    for (i = build_index; i * 2 < copy_to->size; i = child){
      child = i * 2;
      if (child != copy_to->size && copy_to->elements[child + 1] < copy_to->elements[child])
        child++;
      if (copy_to->elements[child] < build_tmp)
        copy_to->elements[i] = copy_to->elements[child];
      else
        break;
    }
    copy_to->elements[i] = build_tmp;
  }

  return copy_to;
}
