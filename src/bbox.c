/*
 * Copyright (c) 2009, NSF Cloud and Autonomic Computing Center, Rutgers University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided
 * that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this list of conditions and
 * the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 * the following disclaimer in the documentation and/or other materials provided with the distribution.
 * - Neither the name of the NSF Cloud and Autonomic Computing Center, Rutgers University, nor the names of its
 * contributors may be used to endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
*  Ciprian Docan (2009)  TASSL Rutgers University
*  docan@cac.rutgers.edu
*  Tong Jin (2011) TASSL Rutgers University
*  tjin@cac.rutgers.edu
*/

#include "bbox.h"

//static inline unsigned int 
static inline uint64_t 
coord_dist(struct coord *c0, struct coord *c1, int dim)
{
        return (c1->c[dim] - c0->c[dim] + 1);
}

//int bbox_dist(struct bbox *bb, int dim)
uint64_t bbox_dist(struct bbox *bb, int dim)
{
        return coord_dist(&bb->lb, &bb->ub, dim);
}

/*
  Test if bounding boxes b0 and b1 intersect along dimension dim.
*/
static int bbox_intersect_ondim(const struct bbox *b0, const struct bbox *b1, int dim)
{
        if ((b0->lb.c[dim] <= b1->lb.c[dim] && 
             b1->lb.c[dim] <= b0->ub.c[dim]) || 
            (b1->lb.c[dim] <= b0->lb.c[dim] &&
             b0->lb.c[dim] <= b1->ub.c[dim]))
                return 1;
        else    return 0;
}

size_t str_len(const char *str)
{
    if(str)
        return strlen(str);
    else
        return 0;
}

char *str_append_const(char *str, const char *msg)
{
    int len, fix_str;

    len = str_len(str) + str_len(msg) + 1;
    fix_str = (str == 0);
    str = realloc(str, len);
    if(fix_str)
        *str = '\0';
    if(str)
        strcat(str, msg);

    return str;
}

char *str_append(char *str, char *msg)
{
    str = str_append_const(str, msg);

    free(msg);
    return str;
}

char *alloc_sprintf(const char *fmt_str, ...)
{
    va_list va_args_tmp, va_args;
    int size;
    char *str;
  
    va_start(va_args_tmp, fmt_str);
    va_copy(va_args, va_args_tmp);
    size = vsnprintf(NULL, 0, fmt_str, va_args_tmp);
    va_end(va_args_tmp);
    str = malloc(sizeof(*str) * (size + 1));
    vsprintf(str, fmt_str, va_args); 
    va_end(va_args);

    return(str);

}

/*
  Test if bounding boxes b0 and b1 intersect (on all dimensions).
*/
int bbox_does_intersect(const struct bbox *b0, const struct bbox *b1)
{
    int i;
    //printf("b1 dims=%d, b2 dims=%d\n", b0->num_dims, b1->num_dims);
    //if(b0->num_dims == b1->num_dims){
        for(i = 0; i < b0->num_dims; i++){
            if(!bbox_intersect_ondim(b0, b1, i))
                return 0;
        }
        return 1;
    //}
    //return 0;
}

/*
  Compute the intersection of bounding boxes b0 and b1, and store it on
  b2. Implicit assumption: b0 and b1 intersect.
*/
void bbox_intersect(struct bbox *b0, const struct bbox *b1, struct bbox *b2)
{
        int i;

        b2->num_dims = b0->num_dims;
        for (i = 0; i < b0->num_dims; i++) {
                b2->lb.c[i] = max(b0->lb.c[i], b1->lb.c[i]);
                b2->ub.c[i] = min(b0->ub.c[i], b1->ub.c[i]);
        }
}

/*
  Test if two bounding boxes are equal.
*/
int bbox_equals(const struct bbox *bb0, const struct bbox *bb1)
{
    int i;
    if(bb0->num_dims == bb1->num_dims){
        for(i = 0; i < bb0->num_dims; i++){
            if((bb0->lb.c[i] != bb1->lb.c[i]) ||
               (bb0->ub.c[i] != bb1->ub.c[i]))
                return 0;
        }
        return 1;
    }
    return 0;
}

uint64_t bbox_volume(struct bbox *bb)
{
    uint64_t n = 1;
    int ndims = bb->num_dims;
    int i;

    for(i = 0; i < ndims; i++){
        n = n * coord_dist(&bb->lb, &bb->ub, i);
    }
    return n;
}



void coord_print(struct coord *c, int num_dims)
{
        switch (num_dims) {
        case 3:
                printf("{%" PRIu64 ", %" PRIu64  ", %" PRIu64 "}", c->c[0], c->c[1], c->c[2]);
                break;
        case 2:
                printf("{%" PRIu64 ", %" PRIu64 "}", c->c[0], c->c[1]);
                break;
        case 1:
                printf("{%" PRIu64 "}", c->c[0]);
        }
}

/*
  Routine to return a string representation of the 'coord' object.
*/

char *coord_sprint(const struct coord *c, int num_dims)
{
    char *str;
    int i;
    int size = 2; // count the curly braces

    for(i = 0; i < num_dims; i++) {
        size += snprintf(NULL, 0, "%" PRIu64, c->c[i]);
        if(i > 0) {
        }
        size += i ? 2 : 0; // account for ", " 
    }
    str = malloc(sizeof(*str) * (size + 1)); // add null terminator
    strcpy(str, "{");
    for(i = 0; i < num_dims; i++) {
        char *tmp  = alloc_sprintf(i?", %" PRIu64 :"%" PRIu64, c->c[i]);
        str = str_append(str, tmp);
    }
    str = str_append_const(str, "}");
    
    return str;    
}


void bbox_print(struct bbox *bb)
{
        printf("{lb = ");
        coord_print(&bb->lb, bb->num_dims);
        printf(", ub = ");
        coord_print(&bb->ub, bb->num_dims);
        printf("}");
}

char * bbox_sprint(const struct bbox *bb)
{
    char *str = strdup("{lb = ");
    
    str = str_append(str, coord_sprint(&bb->lb, bb->num_dims));
    str = str_append_const(str, ", ub = ");
    str = str_append(str, coord_sprint(&bb->ub, bb->num_dims));
    str = str_append_const(str, "}\n");

    return str;
}
