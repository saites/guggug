#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "disjoint.h"

#define talloc(type, num) (type *) malloc(sizeof(type)*(num))

DisjointSet *new_disjoint_set(int maxindex)
{
  DisjointSet *dj;
  int i;

  dj = talloc(DisjointSet, 1);
  if (dj == NULL) { perror("malloc"); exit(1); }

  dj->maxindex = maxindex;
  dj->nsets = 0;

  dj->links = talloc(int, maxindex);
  if (dj->links == NULL) { perror("malloc"); exit(1); }
  for (i = 0; i < maxindex; i++) dj->links[i] = -2;

  dj->sizes = talloc(int, maxindex);
  if (dj->sizes == NULL) { perror("malloc"); exit(1); }
  for (i = 0; i < maxindex; i++) dj->sizes[i] = -1;

  dj->ranks = talloc(int, maxindex);
  if (dj->ranks == NULL) { perror("malloc"); exit(1); }
  for (i = 0; i < maxindex; i++) dj->ranks[i] = -1;
  return dj;
}

void free_disjoint_set(DisjointSet *dj)
{
  free(dj->links);
  free(dj->sizes);
  free(dj->ranks);
  free(dj);
}

void disjoint_makeset(DisjointSet *dj, int index)
{
  if (dj->links[index] != -2) {
    fprintf(stderr, "disjoint_makeset called on an already made set %d\n", index);
    exit(1);
  }
  dj->links[index] = -1;
  dj->nsets++;
  dj->sizes[index] = 1;
  dj->ranks[index] = 1;
}

int disjoint_union(DisjointSet *dj, int s1, int s2)
{
  if (dj->links[s1] != -1) {
    fprintf(stderr, "disjoint_union called on a non-root node %d\n", s1);
    exit(1);
  }
  if (dj->links[s2] != -1) {
    fprintf(stderr, "disjoint_union called on a non-root node %d\n", s2);
    exit(1);
  }
  if (s1 == s2) return s1;

  dj->nsets--;

  if (dj->ranks[s1] < dj->ranks[s2]) {
    dj->links[s1] = s2;
    dj->sizes[s2] += dj->sizes[s1];
    return s2;
  } else if (dj->ranks[s1] > dj->ranks[s2]) {
    dj->links[s2] = s1;
    dj->sizes[s1] += dj->sizes[s2];
    return s1;
  } else {
    dj->links[s2] = s1;
    dj->sizes[s1] += dj->sizes[s2];
    dj->ranks[s1]++;
    return s1;
  }
}

int disjoint_find(DisjointSet *dj, int index)
{
  int id;

  if (dj->links[index] == -1) return index;

  if (dj->links[dj->links[index]] == -1) return dj->links[index];

  id = disjoint_find(dj, dj->links[index]);
  dj->links[index] = id;
  return id;
}

