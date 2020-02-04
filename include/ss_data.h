/*
 * Copyright (c) 2020, Rutgers Discovery Informatics Institute, Rutgers University
 *
 * See COPYRIGHT in top-level directory.
 */

#ifndef __SS_DATA_H_
#define __SS_DATA_H_

#include <stdlib.h>

#include "bbox.h"
#include "list.h"
//#include <sys/mman.h>
//#include <fcntl.h>
//#include <sys/stat.h>
#include <mercury.h>
#include <mercury_bulk.h>
#include <mercury_atomic.h>
#include <mercury_macros.h>

#define BBOX_MAX_NDIM 10
#define MAX_VERSIONS 10

typedef struct {
	void			*iov_base;
	size_t			iov_len;
} iovec_t;

enum storage_type {row_major, column_major};

typedef struct{
        char                    name[154];

        enum storage_type       st;

        int                     owner;
        unsigned int            version;

        /* Global bounding box descriptor. */
        struct bbox             bb;

        /* Size of one element of a data object. */
        size_t                  size;
} obj_descriptor;


struct obj_data {
        struct list_head        obj_entry;

        obj_descriptor   obj_desc;
        void                    *data;		/* Aligned pointer */

        /* Reference to the parent object; used only for sub-objects. */
        struct obj_data         *obj_ref;

        /* Count how many references are to this data object. */
        int                     refcnt;

        /* Flag to mark if we should free this data object. */
        unsigned int            f_free:1;
};

typedef struct {
        int                     num_obj;
        int                     size_hash;
        /* List of data objects. */
        struct list_head        obj_hash[1];
} ss_storage;

struct obj_desc_list {
	struct list_head	odsc_entry;
	obj_descriptor	odsc;
};

typedef struct{
        size_t size;
        char *raw_odsc;

} odsc_hdr;

static inline hg_return_t hg_proc_odsc_hdr(hg_proc_t proc, void *arg)
{
  hg_return_t ret;
  odsc_hdr *in = (odsc_hdr*)arg;
  ret = hg_proc_hg_size_t(proc, &in->size);
  if(ret != HG_SUCCESS) return ret;
  if (in->size) {
    switch (hg_proc_get_op(proc)) {
    case HG_ENCODE:
        ret = hg_proc_raw(proc, in->raw_odsc, in->size);
        if(ret != HG_SUCCESS) return ret;
      break;
    case HG_DECODE:
      in->raw_odsc = (char*)malloc(in->size);
      ret = hg_proc_raw(proc, in->raw_odsc, in->size);
      if(ret != HG_SUCCESS) return ret;
      break;
    case HG_FREE:
      free(in->raw_odsc);
      break;
    default:
      break;
    }
  }
  return HG_SUCCESS;
}



MERCURY_GEN_PROC(bulk_in_t,
        ((odsc_hdr)(odsc))\
        ((hg_bulk_t)(handle)))
MERCURY_GEN_PROC(bulk_out_t, ((int32_t)(ret)))

char * obj_desc_sprint(obj_descriptor *);
int ssd_copy(struct obj_data *, struct obj_data *);

ss_storage *ls_alloc(int max_versions);
void ls_free(ss_storage *);
void ls_add_obj(ss_storage *, struct obj_data *);
struct obj_data* ls_lookup(ss_storage *, char *);
void ls_remove(ss_storage *, struct obj_data *);
void ls_try_remove_free(ss_storage *, struct obj_data *);
int ls_find_ods(ss_storage *, obj_descriptor *, struct obj_data **);
struct obj_data * ls_find_no_version(ss_storage *, obj_descriptor *);

struct obj_data *obj_data_alloc(obj_descriptor *);
struct obj_data *obj_data_alloc_with_data(obj_descriptor *, const void *);

void obj_data_free(struct obj_data *od);
uint64_t obj_data_size(obj_descriptor *);

int obj_desc_equals(obj_descriptor *, obj_descriptor *);
int obj_desc_equals_no_owner(obj_descriptor *, obj_descriptor *);

int obj_desc_equals_intersect(obj_descriptor *odsc1,
                obj_descriptor *odsc2);

int obj_desc_by_name_intersect(obj_descriptor *odsc1,
                obj_descriptor *odsc2);

#endif /* __SS_DATA_H_ */
