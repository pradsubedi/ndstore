#include "ss_data.h"
#include "ndstore-client.h"

static enum storage_type st = column_major;

struct ndstore_client {
    margo_instance_id mid;
    hg_id_t ndstore_put_id;
    hg_id_t ndstore_get_id;
    uint64_t num_provider_handles;
};

struct ndstore_provider_handle {
    ndstore_client_t client;
    hg_addr_t      addr;
    uint16_t       provider_id;
    uint64_t       refcount;
};


static int ndstore_client_register(ndstore_client_t client, margo_instance_id mid)
{
    client->mid = mid;

    /* check if RPCs have already been registered */
    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "ndstore_put_rpc", &id, &flag);

    if(flag == HG_TRUE) { /* RPCs already registered */
        margo_registered_name(mid, "ndstore_put_rpc",                   &client->ndstore_put_id,                   &flag);
        margo_registered_name(mid, "ndstore_get_rpc",                   &client->ndstore_get_id,                   &flag);
   
    } else {

        client->ndstore_put_id =
            MARGO_REGISTER(mid, "ndstore_put_rpc", bulk_put_in_t, bulk_put_out_t, NULL);
        client->ndstore_get_id =
            MARGO_REGISTER(mid, "ndstore_get_rpc", bulk_get_in_t, bulk_get_out_t, NULL);
    }

    return NDSTORE_SUCCESS;
}

int ndstore_client_init(margo_instance_id mid, ndstore_client_t* client)
{
    ndstore_client_t c = (ndstore_client_t)calloc(1, sizeof(*c));
    if(!c) return NDSTORE_ERR_ALLOCATION;

    c->num_provider_handles = 0;

    int ret = ndstore_client_register(c, mid);
    if(ret != 0) return ret;

    *client = c;
    return NDSTORE_SUCCESS;
}


int ndstore_client_finalize(ndstore_client_t client)
{
    if(client->num_provider_handles != 0) {
        fprintf(stderr,
                "[NDSTORE] Warning: %d provider handles not released before ndstore_client_finalize was called\n",
                client->num_provider_handles);
    }
    free(client);
    return NDSTORE_SUCCESS;
}

int ndstore_provider_handle_create(
        ndstore_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        ndstore_provider_handle_t* handle)
{
    if(client == NDSTORE_CLIENT_NULL) 
        return NDSTORE_ERR_INVALID_ARG;

    ndstore_provider_handle_t provider =
        (ndstore_provider_handle_t)calloc(1, sizeof(*provider));

    if(!provider) return NDSTORE_ERR_ALLOCATION;

    hg_return_t ret = margo_addr_dup(client->mid, addr, &(provider->addr));
    if(ret != HG_SUCCESS) {
        free(provider);
        return NDSTORE_ERR_MERCURY;
    }

    provider->client      = client;
    provider->provider_id = provider_id;
    provider->refcount    = 1;

    client->num_provider_handles += 1;

    *handle = provider;
    return NDSTORE_SUCCESS;
}


int ndstore_provider_handle_get_info(
        ndstore_provider_handle_t ph,
        ndstore_client_t* client,
        hg_addr_t* addr,
        uint16_t* provider_id) {
    if(ph) {
        if(client) *client = ph->client;
        if(addr) *addr = ph->addr;
        if(provider_id) *provider_id = ph->provider_id;
    } else {
        if(client) *client = NDSTORE_CLIENT_NULL;
        if(addr) *addr = HG_ADDR_NULL;
        if(provider_id) *provider_id = 0;
    }
    return NDSTORE_SUCCESS;
}

int ndstore_provider_handle_ref_incr(
        ndstore_provider_handle_t handle)
{
    if(handle == NDSTORE_PROVIDER_HANDLE_NULL) return NDSTORE_ERR_INVALID_ARG;
    handle->refcount += 1;
    return NDSTORE_SUCCESS;
}

int ndstore_provider_handle_release(ndstore_provider_handle_t handle)
{
    if(handle == NDSTORE_PROVIDER_HANDLE_NULL) return -1;
    handle->refcount -= 1;
    if(handle->refcount == 0) {
        margo_addr_free(handle->client->mid, handle->addr);
        handle->client->num_provider_handles -= 1;
        free(handle);
    }
    return NDSTORE_SUCCESS;
}


int ndstore_put (ndstore_provider_handle_t provider,
		const char *var_name,
        unsigned int ver, int elem_size,
        int ndim, uint64_t *lb, uint64_t *ub, 
        const void *data)
{
	hg_return_t hret;
    int ret = NDSTORE_SUCCESS;
    hg_handle_t handle;

    /*****/

    obj_descriptor odsc = {
            .version = ver, .owner = -1, 
            .st = st,
            .size = elem_size,
            .bb = {.num_dims = ndim,}
    };

    memset(odsc.bb.lb.c, 0, sizeof(uint64_t)*BBOX_MAX_NDIM);
    memset(odsc.bb.ub.c, 0, sizeof(uint64_t)*BBOX_MAX_NDIM);

    memcpy(odsc.bb.lb.c, lb, sizeof(uint64_t)*ndim);
    memcpy(odsc.bb.ub.c, ub, sizeof(uint64_t)*ndim);

    strncpy(odsc.name, var_name, sizeof(odsc.name)-1);
    odsc.name[sizeof(odsc.name)-1] = '\0';


    bulk_put_in_t in;
    bulk_put_out_t out;

    in.odsc = odsc;
    in.size = (odsc.size)*bbox_volume(&(odsc.bb));

    hret = margo_bulk_create(provider->client->mid, 1, (void**)(&data), &in.size,
                            HG_BULK_READ_ONLY, &in.handle);
    if(hret != HG_SUCCESS) {
        fprintf(stderr,"[NDSTORE] margo_bulk_create() failed in ndstore_put()\n");
        return NDSTORE_ERR_MERCURY;
    }

    /* create handle */
    hret = margo_create(
            provider->client->mid,
            provider->addr,
            provider->client->ndstore_put_id,
            &handle);
    if(hret != HG_SUCCESS) {
        fprintf(stderr,"[NDSTORE] margo_create() failed in ndstore_put()\n");
        margo_bulk_free(in.handle);
        return NDSTORE_ERR_MERCURY;
    }

    hret = margo_provider_forward(provider->provider_id, handle, &in);
    if(hret != HG_SUCCESS) {
        fprintf(stderr,"[NDSTORE] margo_forward() failed in ndstore_put()\n");
        margo_bulk_free(in.handle);
        margo_destroy(handle);
        return NDSTORE_ERR_MERCURY;
    }

    hret = margo_get_output(handle, &out);
    if(hret != HG_SUCCESS) {
        fprintf(stderr,"[NDSTORE] margo_get_output() failed in ndstore_put()\n");
        margo_bulk_free(in.handle);
        margo_destroy(handle);
        return NDSTORE_ERR_MERCURY;
    }

    ret = out.ret;
    margo_free_output(handle, &out);
    margo_bulk_free(in.handle);

    margo_destroy(handle);
	return ret;

}

int ndstore_get (ndstore_provider_handle_t provider,
		const char *var_name,
        unsigned int ver, int elem_size,
        int ndim, uint64_t *lb, uint64_t *ub,
        const void *data)
{
	hg_return_t hret;
    int ret = NDSTORE_SUCCESS;
    hg_handle_t handle;

    /*****/

    obj_descriptor odsc = {
            .version = ver, .owner = -1,
            .st = st,
            .size = elem_size,
            .bb = {.num_dims = ndim,}
    };

    memset(odsc.bb.lb.c, 0, sizeof(uint64_t)*BBOX_MAX_NDIM);
    memset(odsc.bb.ub.c, 0, sizeof(uint64_t)*BBOX_MAX_NDIM);

    memcpy(odsc.bb.lb.c, lb, sizeof(uint64_t)*ndim);
    memcpy(odsc.bb.ub.c, ub, sizeof(uint64_t)*ndim);

    strncpy(odsc.name, var_name, sizeof(odsc.name)-1);
    odsc.name[sizeof(odsc.name)-1] = '\0';


    bulk_get_in_t in;
    bulk_get_out_t out;

    in.odsc = odsc;
    hg_size_t size = (odsc.size)*bbox_volume(&(odsc.bb));

    hret = margo_bulk_create(provider->client->mid, 1, (void**)(&data), &size,
                            HG_BULK_WRITE_ONLY, &in.handle);
    if(hret != HG_SUCCESS) {
        fprintf(stderr,"[NDSTORE] margo_bulk_create() failed in ndstore_put()\n");
        return NDSTORE_ERR_MERCURY;
    }

    /* create handle */
    hret = margo_create(
            provider->client->mid,
            provider->addr,
            provider->client->ndstore_put_id,
            &handle);
    if(hret != HG_SUCCESS) {
        fprintf(stderr,"[NDSTORE] margo_create() failed in ndstore_put()\n");
        margo_bulk_free(in.handle);
        return NDSTORE_ERR_MERCURY;
    }

    hret = margo_provider_forward(provider->provider_id, handle, &in);
    if(hret != HG_SUCCESS) {
        fprintf(stderr,"[NDSTORE] margo_forward() failed in ndstore_put()\n");
        margo_bulk_free(in.handle);
        margo_destroy(handle);
        return NDSTORE_ERR_MERCURY;
    }

    hret = margo_get_output(handle, &out);
    if(hret != HG_SUCCESS) {
        fprintf(stderr,"[NDSTORE] margo_get_output() failed in ndstore_put()\n");
        margo_bulk_free(in.handle);
        margo_destroy(handle);
        return NDSTORE_ERR_MERCURY;
    }

    ret = out.ret;
    margo_free_output(handle, &out);
    margo_bulk_free(in.handle);

    margo_destroy(handle);
	return ret;

}
