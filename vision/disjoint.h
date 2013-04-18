#ifndef _DISJOINT_
#define _DISJOINT_

typedef struct {
  int *links;
  int *sizes;
  int *ranks;
  int maxindex;
  int nsets;
} DisjointSet;
  
extern DisjointSet *new_disjoint_set(int maxindex);
extern void free_disjoint_set(DisjointSet *dj);
extern void disjoint_makeset(DisjointSet *dj, int index);
extern int disjoint_union(DisjointSet *dj, int s1, int s2);
extern int disjoint_find(DisjointSet *dj, int index);

#endif
