/*
 * Copyright (c) 2020, Rutgers Discovery Informatics Institute, Rutgers University
 *
 * See COPYRIGHT in top-level directory.
 */

#include "ss_data.h"
#include "ndstore-server.h"

static enum storage_type st = column_major;

struct ndstore_provider{
    margo_instance_id mid;
    hg_id_t ndstore_put_id;
    hg_id_t ndstore_get_id;
    ss_storage *ls;

};


DECLARE_MARGO_RPC_HANDLER(ndstore_put_ult);
DECLARE_MARGO_RPC_HANDLER(ndstore_get_ult);

static void ndstore_put_ult(hg_handle_t h);
static void ndstore_get_ult(hg_handle_t h);

static void ndstore_finalize_provider(void* p);

int ndstore_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        ABT_pool pool,
        ndstore_provider_t* provider)
{
	ndstore_provider_t server;
    int ret;


    /* check if a provider with the same id already exists */
    {
        hg_id_t id;
        hg_bool_t flag;

        flag = margo_is_listening(mid);
	    if(flag == HG_FALSE) {
	        fprintf(stderr, "ndstore_provider_register(): margo instance is not a server.");
	        return NDSTORE_ERR_INVALID_ARG;
	    }
        margo_provider_registered_name(mid, "ndstore_put_rpc", provider_id, &id, &flag);
        if(flag == HG_TRUE) {
            fprintf(stderr, "ndstore_provider_register(): a provider with the same provider id (%d) already exists\n", provider_id);
            return NDSTORE_ERR_MERCURY;
        }
    }


    server = (ndstore_provider_t)calloc(1, sizeof(*server));
    if(server == NULL)
        return NDSTORE_ERR_ALLOCATION;

    server->mid = mid;
    hg_id_t rpc_id;
    rpc_id = MARGO_REGISTER_PROVIDER(mid, "ndstore_put_rpc",
            bulk_in_t, bulk_out_t,
            ndstore_put_ult, provider_id, pool);
    margo_register_data(mid, rpc_id, (void*)server, NULL);
    server->ndstore_put_id = rpc_id;

    rpc_id = MARGO_REGISTER_PROVIDER(mid, "ndstore_get_rpc",
            bulk_in_t, bulk_out_t,
            ndstore_get_ult, provider_id, pool);
    margo_register_data(mid, rpc_id, (void*)server, NULL);
    server->ndstore_get_id = rpc_id;
    /* add other RPC registration here */

    server->ls = ls_alloc(MAX_VERSIONS);
    if (!server->ls) {
            fprintf(stderr, "ndstore_provider_register(): Error Local storage allcation failed\n");
            ndstore_provider_destroy(server);
            return NDSTORE_ERR_ALLOCATION;
        }

    margo_provider_push_finalize_callback(mid, server, &ndstore_finalize_provider, server);

    *provider = server;
    return NDSTORE_SUCCESS;
}

static void ndstore_finalize_provider(void* p)
{
    ndstore_provider_t provider = (ndstore_provider_t)p;
    margo_instance_id mid = provider->mid;

    margo_deregister(mid, provider->ndstore_put_id);
    margo_deregister(mid, provider->ndstore_get_id);
    /* deregister other RPC ids ... */
    ls_free(provider->ls);
    free(provider);
}

int ndstore_provider_destroy(
        ndstore_provider_t provider)
{
    /* pop the finalize callback */
    margo_provider_pop_finalize_callback(provider->mid, provider);
    /* call the callback */
    ndstore_finalize_provider(provider);

    return NDSTORE_SUCCESS;
}



static void ndstore_put_ult(hg_handle_t handle)
{
    hg_return_t hret;
    bulk_in_t in;
    bulk_out_t out;
    hg_bulk_t bulk_handle;

    margo_instance_id mid = margo_hg_handle_get_instance(handle);

    const struct hg_info* info = margo_get_info(handle);
    ndstore_provider_t provider = (ndstore_provider_t)margo_registered_data(mid, info->id);

     if(!provider) {
        fprintf(stderr, "Error (ndstore_put_ult): NDSTORE could not find provider\n"); 
        out.ret = NDSTORE_ERR_UNKNOWN_PR;
        margo_respond(handle, &out);
        margo_destroy(handle);
        return;
    }

    hret = margo_get_input(handle, &in);
    if(hret != HG_SUCCESS) {
        out.ret = NDSTORE_ERR_MERCURY;
        margo_respond(handle, &out);
        margo_destroy(handle);
        return;
    }

    obj_descriptor in_odsc;
    memcpy(&in_odsc, in.odsc.raw_odsc, sizeof(in_odsc));

    struct obj_data *od;
    od = obj_data_alloc(&in_odsc);
    if(!od)
        fprintf(stderr, "Obj_data_alloc error\n");
    hg_size_t size = (in_odsc.size)*bbox_volume(&(in_odsc.bb));

    void *buffer = (void*) od->data;
    hret = margo_bulk_create(mid, 1, (void**)&(od->data), &size,
                HG_BULK_WRITE_ONLY, &bulk_handle);

    if(hret != HG_SUCCESS) {
        fprintf(stderr, "Error in margo_bulk_create\n");
        out.ret = NDSTORE_ERR_MERCURY;
        margo_respond(handle, &out);
        margo_free_input(handle, &in);
        margo_destroy(handle);
        return;
	}
    
    hret = margo_bulk_transfer(mid, HG_BULK_PULL, info->addr, in.handle, 0,
            bulk_handle, 0, size);
    if(hret != HG_SUCCESS) {
        fprintf(stderr, "Error in margo_bulk_transfer\n");
        out.ret = NDSTORE_ERR_MERCURY;
        margo_respond(handle, &out);
        margo_free_input(handle, &in);
        margo_bulk_free(bulk_handle);
        margo_destroy(handle);
        return;
    }

    out.ret = NDSTORE_SUCCESS;
    ls_add_obj(provider->ls, od);

    margo_bulk_free(bulk_handle);
    margo_respond(handle, &out);
    margo_free_input(handle, &in);
    margo_destroy(handle);
}
DEFINE_MARGO_RPC_HANDLER(ndstore_put_ult)


static void ndstore_get_ult(hg_handle_t handle)
{
    hg_return_t hret;
    bulk_in_t in;
    bulk_out_t out;
    hg_bulk_t bulk_handle;

    margo_instance_id mid = margo_hg_handle_get_instance(handle);

    const struct hg_info* info = margo_get_info(handle);
    ndstore_provider_t provider = (ndstore_provider_t)margo_registered_data(mid, info->id);

     if(!provider) {
        fprintf(stderr, "Error (ndstore_put_ult): NDSTORE could not find provider\n"); 
        out.ret = NDSTORE_ERR_UNKNOWN_PR;
        margo_respond(handle, &out);
        margo_destroy(handle);
        return;
    }


    hret = margo_get_input(handle, &in);
    if(hret != HG_SUCCESS) {
        out.ret = NDSTORE_ERR_MERCURY;
        margo_respond(handle, &out);
        margo_destroy(handle);
        return;
    }

    obj_descriptor in_odsc;
    memcpy(&in_odsc, in.odsc.raw_odsc, sizeof(in_odsc));
     
    struct obj_data *od, *from_obj;
    struct obj_data **od_tab;
    od_tab = malloc(sizeof(*od_tab) * provider->ls->num_obj);

    int obj_nums = 0;
    obj_nums = ls_find_ods(provider->ls, &in_odsc, od_tab);

    if (obj_nums == 0) {
        char *str;
        str = obj_desc_sprint(&in_odsc);
        fprintf(stderr, "Error (ndstore_put_ult): No objects found for %s", str);
        free(str);
        out.ret = NDSTORE_ERR_UNKNOWN_OBJ;
        margo_respond(handle, &out);
        margo_free_input(handle, &in);
        margo_destroy(handle);
        return;
    }

    od = obj_data_alloc(&in_odsc);
    int i, total_elems_found;
    total_elems_found = 0;
    for(i=0; i<obj_nums; i++){
        total_elems_found += ssd_copy(od, od_tab[i]);
    }
    free(od_tab);

    if(total_elems_found!=bbox_volume(&(in_odsc.bb))){
        out.ret = NDSTORE_ERR_UNKNOWN_OBJ;
        fprintf(stderr, "Error (ndstore_put_ult): Only partial objecyt is found. Returning Error to the client\n");
        obj_data_free(od);
        margo_respond(handle, &out);
        margo_free_input(handle, &in);
        margo_destroy(handle);
        return;
    }
    
    hg_size_t size = (in_odsc.size)*bbox_volume(&(in_odsc.bb));
    void *buffer = (void*) od->data;
    hret = margo_bulk_create(mid, 1, (void**)&buffer, &size,
                HG_BULK_READ_ONLY, &bulk_handle);

    if(hret != HG_SUCCESS) {
        fprintf(stderr,"Error in margo_bulk_create()\n");
        out.ret = NDSTORE_ERR_MERCURY;
        margo_respond(handle, &out);
        margo_free_input(handle, &in);
        margo_destroy(handle);
        return;
	}

    hret = margo_bulk_transfer(mid, HG_BULK_PUSH, info->addr, in.handle, 0,
            bulk_handle, 0, size);
    if(hret != HG_SUCCESS) {
        fprintf(stderr,"Error in margo_bulk_transfer()\n");
        out.ret = NDSTORE_ERR_MERCURY;
        margo_respond(handle, &out);
        margo_free_input(handle, &in);
        margo_bulk_free(bulk_handle);
        margo_destroy(handle);
        return;
    }
    margo_bulk_free(bulk_handle);
    out.ret = NDSTORE_SUCCESS;
    obj_data_free(od);
    margo_respond(handle, &out);
    margo_free_input(handle, &in);
    margo_destroy(handle);
}
DEFINE_MARGO_RPC_HANDLER(ndstore_get_ult)
