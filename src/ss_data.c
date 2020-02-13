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
*
*  Pradeep Subedi (2020) Rutgers University
*  pradeep.subedi@rutgers.edu
*/

#include <math.h>
#include <errno.h>
#include "ss_data.h"


/*
  A view in  the matrix allows to extract any subset  of values from a
  matrix.
*/


struct matrix_view {
        uint64_t   lb[BBOX_MAX_NDIM];
        uint64_t   ub[BBOX_MAX_NDIM];	
};

/* Generic matrix representation. */
struct matrix {
        uint64_t   dist[BBOX_MAX_NDIM];
        int 			        num_dims;
        size_t                  size_elem;
        enum storage_type       mat_storage;
        struct matrix_view      mat_view;
        void                    *pdata;
};

static void matrix_init(struct matrix *mat, enum storage_type st,
                        struct bbox *bb_glb, struct bbox *bb_loc, 
                        void *pdata, size_t se)
{
    int i;
    int ndims = bb_glb->num_dims;
    memset(mat, 0, sizeof(struct matrix));

    for(i = 0; i < ndims; i++){
        mat->dist[i] = bbox_dist(bb_glb, i);
        mat->mat_view.lb[i] = bb_loc->lb.c[i] - bb_glb->lb.c[i];
        mat->mat_view.ub[i] = bb_loc->ub.c[i] - bb_glb->lb.c[i];
    }

    mat->num_dims = ndims;
    mat->mat_storage = st;
    mat->pdata = pdata;
    mat->size_elem = se;
}

static int matrix_copy(struct matrix *a, struct matrix *b)
{
        char *A = a->pdata;
        char *B = b->pdata;

        uint64_t a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
        uint64_t aloc=0, aloc1=0, aloc2=0, aloc3=0, aloc4=0, aloc5=0, aloc6=0, aloc7=0, aloc8=0, aloc9=0;
        uint64_t b0, b1, b2, b3, b4, b5, b6, b7, b8, b9;
        uint64_t bloc=0, bloc1=0, bloc2=0, bloc3=0, bloc4=0, bloc5=0, bloc6=0, bloc7=0, bloc8=0, bloc9=0;
    uint64_t numelem;
    uint64_t num_copied_elem = 0;

    switch(a->num_dims){
        case(1):    
            goto dim1;
            break;
        case(2):
            goto dim2;
            break;
        case(3):
            goto dim3;
            break;
        case(4):
            goto dim4;
            break;
        case(5):
            goto dim5;
            break;
        case(6):
            goto dim6;
            break;
        case(7):
            goto dim7;
            break;
        case(8):
            goto dim8;
            break;
        case(9):
            goto dim9;
            break;
        case(10):
            goto dim10;
            break;
        default:
            break;
    }
    
dim10:        for(a9 = a->mat_view.lb[9], b9 = b->mat_view.lb[9];   //TODO-Q
            a9 <= a->mat_view.ub[9]; a9++, b9++){
            aloc9 = a9 * a->dist[8];
            bloc9 = a9 * b->dist[8];
dim9:        for(a8 = a->mat_view.lb[8], b8 = b->mat_view.lb[8];    //TODO-Q
            a8 <= a->mat_view.ub[8]; a8++, b8++){
            aloc8 = (aloc9 + a8) * a->dist[7];
            bloc8 = (bloc9 + b8) * b->dist[7];
dim8:        for(a7 = a->mat_view.lb[7], b7 = b->mat_view.lb[7];    //TODO-Q
            a7 <= a->mat_view.ub[7]; a7++, b7++){
            aloc7 = (aloc8 + a7) * a->dist[6];
            bloc7 = (bloc8 + b7) * b->dist[6];
dim7:        for(a6 = a->mat_view.lb[6], b6 = b->mat_view.lb[6];    //TODO-Q
            a6 <= a->mat_view.ub[6]; a6++, b6++){
            aloc6 = (aloc7 + a6) * a->dist[5];
            bloc6 = (bloc7 + b6) * b->dist[5];
dim6:        for(a5 = a->mat_view.lb[5], b5 = b->mat_view.lb[5];    //TODO-Q
            a5 <= a->mat_view.ub[5]; a5++, b5++){
            aloc5 = (aloc6 + a5) * a->dist[4];
            bloc5 = (bloc6 + b5) * b->dist[4];
dim5:        for(a4 = a->mat_view.lb[4], b4 = b->mat_view.lb[4];
            a4 <= a->mat_view.ub[4]; a4++, b4++){
            aloc4 = (aloc5 + a4) * a->dist[3];
            bloc4 = (bloc5 + b4) * b->dist[3];
dim4:        for(a3 = a->mat_view.lb[3], b3 = b->mat_view.lb[3];
            a3 <= a->mat_view.ub[3]; a3++, b3++){
            aloc3 = (aloc4 + a3) * a->dist[2];
            bloc3 = (bloc4 + b3) * b->dist[2];
dim3:            for(a2 = a->mat_view.lb[2], b2 = b->mat_view.lb[2];
                a2 <= a->mat_view.ub[2]; a2++, b2++){
                aloc2 = (aloc3 + a2) * a->dist[1];
                bloc2 = (bloc3 + b2) * b->dist[1];
dim2:                for(a1 = a->mat_view.lb[1], b1 = b->mat_view.lb[1];
                    a1 <= a->mat_view.ub[1]; a1++, b1++){
                    aloc1 = (aloc2 + a1) * a->dist[0];
                    bloc1 = (bloc2 + b1) * b->dist[0];
dim1:               numelem = (a->mat_view.ub[0] - a->mat_view.lb[0]) + 1;
                    aloc = aloc1 + a->mat_view.lb[0];
                    bloc = bloc1 + b->mat_view.lb[0];
                    memcpy(&A[aloc*a->size_elem], &B[bloc*a->size_elem], (a->size_elem * numelem));
                    num_copied_elem += numelem;     
            if(a->num_dims == 1)    return num_copied_elem;
                }
        if(a->num_dims == 2)    return num_copied_elem;
            }
        if(a->num_dims == 3)    return num_copied_elem; 
        }
    if(a->num_dims == 4)    return num_copied_elem;
    }
    if(a->num_dims == 5)    return num_copied_elem;
    }
    if(a->num_dims == 6)    return num_copied_elem;
    }
    if(a->num_dims == 7)    return num_copied_elem;
    }
    if(a->num_dims == 8)    return num_copied_elem;
    }
    if(a->num_dims == 9)    return num_copied_elem;
    }
}

char *obj_desc_sprint(obj_descriptor *odsc)
{
    char *str;
    int nb;

    str = alloc_sprintf("obj_descriptor = {\n"
                "\t.name = %s,\n"
                "\t.owner = %d,\n"
                "\t.version = %d\n"
                "\t.bb = ", odsc->name, odsc->owner, odsc->version);
    str = str_append_const(str_append(str, bbox_sprint(&odsc->bb)), "}\n");

    return str;
}
/*
*/
int ssd_copy(struct obj_data *to_obj, struct obj_data *from_obj)
{
        struct matrix to_mat, from_mat;
        struct bbox bbcom;
        int copied_elems = 0;

        bbox_intersect(&to_obj->obj_desc.bb, &from_obj->obj_desc.bb, &bbcom);

        matrix_init(&from_mat, from_obj->obj_desc.st,
                    &from_obj->obj_desc.bb, &bbcom, 
                    from_obj->data, from_obj->obj_desc.size);

        matrix_init(&to_mat, to_obj->obj_desc.st, 
                    &to_obj->obj_desc.bb, &bbcom,
                    to_obj->data, to_obj->obj_desc.size);

        copied_elems = matrix_copy(&to_mat, &from_mat);
        return copied_elems;
}


/*
  Allocate and init the local storage structure.
*/
ss_storage *ls_alloc(int max_versions)
{
        ss_storage *ls = 0;
        int i;

        ls = malloc(sizeof(*ls) + sizeof(struct list_head) * max_versions);
        if (!ls) {
                errno = ENOMEM;
                return ls;
        }

        memset(ls, 0, sizeof(*ls));
        for (i = 0; i < max_versions; i++)
                INIT_LIST_HEAD(&ls->obj_hash[i]);
        ls->size_hash = max_versions;

        return ls;
}

void ls_free(ss_storage *ls)
{
    if (!ls) return;

    struct obj_data *od, *t;
    struct list_head *list;
    int i;

    for (i = 0; i < ls->size_hash; i++) {
        list = &ls->obj_hash[i];
        list_for_each_entry_safe(od, t, list, struct obj_data, obj_entry ) {
            ls_remove(ls, od);
            obj_data_free(od);
        }
    }

    if (ls->num_obj != 0) {
        fprintf(stderr, "%s(): ERROR ls->num_obj is %d not 0\n", __func__, ls->num_obj);
    }
    free(ls);
}

/*
  Add an object to the local storage.
*/
void ls_add_obj(ss_storage *ls, struct obj_data *od)
{
        int index;
        struct list_head *bin;
        struct obj_data *od_existing;

        od_existing = ls_find_no_version(ls, &od->obj_desc);
        if (od_existing) {

            //update here to send rpc requests to inititate rpc call to update local object descriptor
                od_existing->f_free = 1;
                if (od_existing->refcnt == 0) {
                        ls_remove(ls, od_existing);
                        obj_data_free(od_existing);
                }
                else {
                        fprintf(stderr, "'%s()': object eviction delayed.\n", __func__);
                }
        }
        index = od->obj_desc.version % ls->size_hash;
        bin = &ls->obj_hash[index];

        /* NOTE: new object comes first in the list. */
        list_add(&od->obj_entry, bin);
        ls->num_obj++;
}

struct obj_data* ls_lookup(ss_storage *ls, char *name)
{
        struct obj_data *od;
        struct list_head *list;
        int i;

        for (i = 0; i < ls->size_hash; i++) {
                list = &ls->obj_hash[i];

                list_for_each_entry(od, list, struct obj_data, obj_entry ) {
                        if (strcmp(od->obj_desc.name, name) == 0)
                                return od;
                }
        }

        return NULL;
}

void ls_remove(ss_storage *ls, struct obj_data *od)
{
        list_del(&od->obj_entry);
        ls->num_obj--;
}

void ls_try_remove_free(ss_storage *ls, struct obj_data *od)
{
        /* Note:  we   assume  the  object  data   is  allocated  with
           obj_data_alloc(), i.e., the data follows the structure.  */
        if (od->refcnt == 0) {
                ls_remove(ls, od);
                if (od->data != od+1) {
                        fprintf(stderr, "'%s()': we are about to free an object " 
                              "with external allocation.\n", __func__);
                }
                obj_data_free(od);
        }
}

/*
  Find  list of object_desriptors  in the  local storage  that has  the same  name and
  version with the object descriptor 'odsc'.
*/
int ls_find_ods(ss_storage *ls, obj_descriptor *odsc, struct obj_data **od_tab)
{
        struct obj_data *od;
        struct list_head *list;
        int index;
        int num_odsc = 0;

        index = odsc->version % ls->size_hash;
        list = &ls->obj_hash[index];
        list_for_each_entry(od, list, struct obj_data, obj_entry) {
            if (obj_desc_equals_intersect(odsc, &od->obj_desc)){
                od_tab[num_odsc++] = od;
            }
        }
        return num_odsc;
}

/*
  Search for an object in the local storage that is mapped to the same
  bin, and that has the same  name and object descriptor, but may have
  different version.
*/
struct obj_data *
ls_find_no_version(ss_storage *ls, obj_descriptor *odsc)
{
        struct obj_data *od;
        struct list_head *list;
        int index;

        index = odsc->version % ls->size_hash; 
        list = &ls->obj_hash[index];

        list_for_each_entry(od, list, struct obj_data, obj_entry) {
                if (obj_desc_by_name_intersect(odsc, &od->obj_desc))
                        return od;
        }

        return NULL;
}




#define ALIGN_ADDR_QUAD_BYTES(a)                                \
        unsigned long _a = (unsigned long) (a);                 \
        _a = (_a + 7) & ~7;                                     \
        (a) = (void *) _a;
/*
  Allocate space for an obj_data structure and the data.
*/
struct obj_data *obj_data_alloc(obj_descriptor *odsc)
{
    struct obj_data *od = 0;

	od = malloc(sizeof(*od));
	if (!od){
        fprintf(stderr, "Malloc od error\n");
		return NULL;
    }
	memset(od, 0, sizeof(*od));

    int size= obj_data_size(odsc);
    od->data= malloc(size);
    if (!od->data) {
        fprintf(stderr, "Malloc od_data error\n");
        free(od);
        return NULL;
    }
	ALIGN_ADDR_QUAD_BYTES(od->data);
	od->obj_desc = *odsc;

    return od;
}



struct obj_data *obj_data_alloc_with_data(obj_descriptor *odsc, const void *data)
{
        struct obj_data *od = obj_data_alloc(odsc);
        if (!od)
                return NULL;

        memcpy(od->data, data, obj_data_size(odsc));
        //TODO: what about the descriptor ?

        return od;
}



void obj_data_free(struct obj_data *od)
{
    if(od){
        if(od->data){
        	free(od->data);
        }
    	free(od);
    }
}



uint64_t obj_data_size(obj_descriptor *obj_desc)
{
    return obj_desc->size * bbox_volume(&obj_desc->bb);
}


int obj_desc_equals_no_owner(obj_descriptor *odsc1,
                 obj_descriptor *odsc2)
{
        /* Note: object distribution should not change with
           version. */
        if (// odsc1->version == odsc2->version && 
            strcmp(odsc1->name, odsc2->name) == 0 &&
            bbox_equals(&odsc1->bb, &odsc2->bb))
                return 1;
        return 0;
}

int obj_desc_equals(obj_descriptor *odsc1, 
                obj_descriptor *odsc2)
{
        if (odsc1->owner == odsc2->owner && 
            bbox_equals(&odsc1->bb, &odsc2->bb))
                return 1;
        else    return 0;
}

/*
 *   Test if two object descriptors have the same name and versions and
 *     their bounding boxes intersect.
 *     */
int obj_desc_equals_intersect(obj_descriptor *odsc1,
                obj_descriptor *odsc2)
{
        if (strcmp(odsc1->name, odsc2->name) == 0 &&
            odsc1->version == odsc2->version &&
            bbox_does_intersect(&odsc1->bb, &odsc2->bb))
                return 1;
        return 0;
}

/*
 *   Test if two object descriptors have the same name and their bounding
 *     boxes intersect.
 *     */
int obj_desc_by_name_intersect(obj_descriptor *odsc1,
                obj_descriptor *odsc2)
{
        if (strcmp(odsc1->name, odsc2->name) == 0 &&
            bbox_does_intersect(&odsc1->bb, &odsc2->bb))
                return 1;
        return 0;
}



