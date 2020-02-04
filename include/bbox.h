/*
 * Copyright (c) 2020, Rutgers Discovery Informatics Institute, Rutgers University
 *
 * See COPYRIGHT in top-level directory.
 */

#ifndef __BBOX_H_
#define __BBOX_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define max(a,b) (a) > (b) ? (a):(b)
#define min(a,b) (a) < (b) ? (a):(b)
#define BBOX_MAX_NDIM 10

/*
typedef unsigned char 		__u8;
typedef unsigned int 		__u32;
typedef int			__s32;
typedef uint64_t	__u64; //Commented since not all compiler support typedef
*/

enum bb_dim {
        bb_x = 0,
        bb_y = 1,
        bb_z = 2
};

struct coord {
        uint64_t c[BBOX_MAX_NDIM];
};

struct bbox {
        int num_dims;
        struct coord lb, ub;
};

struct intv {
        uint64_t lb, ub;
};

char *str_append_const(char *, const char *);
char *str_append(char *, char *);
char *alloc_sprintf(const char *fmt_str, ...);

uint64_t bbox_dist(struct bbox *, int);
int bbox_include(const struct bbox *, const struct bbox *);
int bbox_does_intersect(const struct bbox *, const struct bbox *);
void bbox_intersect(struct bbox *, const struct bbox *, struct bbox *);
int bbox_equals(const struct bbox *, const struct bbox *);

uint64_t bbox_volume(struct bbox *);
int intv_do_intersect(struct intv *, struct intv *);
uint64_t intv_size(struct intv *);


char *str_append_const(char *, const char *);
char *str_append(char *, char *);

void coord_print(struct coord *c, int num_dims);
char * coord_sprint(const struct coord *c, int num_dims);
void bbox_print(struct bbox *bb);
char * bbox_sprint(const struct bbox *bb);

#endif /* __BBOX_H_ */
