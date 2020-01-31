/*
 * Copyright (c) 2020, Rutgers Discovery Informatics Institute, Rutgers University
 *
 * See COPYRIGHT in top-level directory.
 */


#ifndef __NDSTORE_CLIENT_H
#define __NDSTORE_CLIENT_H

#include <margo.h>
#include <ndstore-common.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct ndstore_client* ndstore_client_t;
#define NDSTORE_CLIENT_NULL ((ndstore_client_t)NULL)

typedef struct ndstore_provider_handle *ndstore_provider_handle_t;
#define NDSTORE_PROVIDER_HANDLE_NULL ((ndstore_provider_handle_t)NULL)

/**
 * @brief Creates a NDSTORE client.
 *
 * @param[in] mid Margo instance
 * @param[out] client NDSTORE client
 *
 * @return NDSTORE_SUCCESS or error code defined in ndstore-common.h
 */
int ndstore_client_init(margo_instance_id mid, ndstore_client_t* client);

/**
 * @brief Finalizes a NDSTORE client.
 *
 * @param[in] client NDSTORE client to finalize
 *
 * @return NDSTORE_SUCCESS or error code defined in ndstore-common.h
 */
int ndstore_client_finalize(ndstore_client_t client);

/**
 * @brief Creates a NDSTORE provider handle.
 *
 * @param[in] client NDSTORE client responsible for the provider handle
 * @param[in] addr Mercury address of the provider
 * @param[in] provider_id id of the provider
 * @param[in] handle provider handle
 *
 * @return NDSTORE_SUCCESS or error code defined in ndstore-common.h
 */
int ndstore_provider_handle_create(
        ndstore_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        ndstore_provider_handle_t* handle);

/**
 * @brief Retrieves the information (string address and provider id)
 * from a provider handle. If any argument is NULL, the corresponding
 * field will be ignored.
 *
 * @param[in] ph Provider handle.
 * @param[inout] client Client.
 * @param[inout] addr Address.
 * @param[inout] provider_id 
 *
 * @return NDSTORE_SUCCESS or error code defined in ndstore-common.h
 */
int ndstore_provider_handle_get_info(
        ndstore_provider_handle_t ph,
        ndstore_client_t* client,
        hg_addr_t* addr,
        uint16_t* provider_id);


/**
 * @brief Increments the reference counter of a provider handle.
 *
 * @param handle provider handle
 *
 * @return NDSTORE_SUCCESS or error code defined in ndstore-common.h
 */
int ndstore_provider_handle_ref_incr(
        ndstore_provider_handle_t handle);

/**
 * @brief Releases the provider handle. This will decrement the
 * reference counter, and free the provider handle if the reference
 * counter reaches 0.
 *
 * @param[in] handle provider handle to release.
 *
 * @return NDSTORE_SUCCESS or error code defined in ndstore-common.h
 */
int ndstore_provider_handle_release(ndstore_provider_handle_t handle);

/**
 * @brief Query the space to insert data specified by a geometric
 *    descriptor.
 * 
 * Memory buffer pointed by pointer "data" is a sub-region of the
 * global n-dimensional array in user application, which is described
 * by the local bounding box {(lb[0],lb[1],..,lb[n-1]), (ub[0],ub[1],..,ub[n-1])}.
 *
 * This routine is non-blocking, and successful return of the routine does not 
 * guarantee the completion of data transfer from client process to dataspaces 
 * staging server.
 *
 * Note: ordering of dimension (fast->slow) is 0, 1, ..., n-1. For C row-major
 * array, the dimensions need to be reordered to construct the bounding box. For
 * example, the bounding box for C array c[2][4] is lb: {0,0}, ub: {3,1}. 
 * 
 * @param[in] var_name:     Name of the variable.
 * @param[in] ver:      Version of the variable.
 * @param[in] size:     Size (in bytes) for each element of the global
 *              array.
 * @param[in] ndim:     the number of dimensions for the local bounding
 *              box. 
 * @param[in] lb:       coordinates for the lower corner of the local
 *                  bounding box.
 * @param[in] ub:       coordinates for the upper corner of the local
 *                  bounding box. 
 * @param[in] data:     Pointer to user data buffer. 
 *
 * @return  0 indicates success.
 */
int ndstore_put (ndstore_provider_handle_t provider,
		const char *var_name,
        unsigned int ver, int size,
        int ndim, uint64_t *lb, uint64_t *ub, 
        const void *data); 

/**
 * @brief Query the space to get data specified by a geometric
 *    descriptor.
 * 
 * Memory buffer pointed by pointer "data" is a sub-region of the
 * global n-dimensional array in user application, which is described
 * by the local bounding box {(lb[0],lb[1],..,lb[n-1]), (ub[0],ub[1],..,ub[n-1])}.
 *
 * This routine is non-blocking, and successful return of the routine does not 
 * guarantee the completion of data transfer from client process to dataspaces 
 * staging server. User applications need to call dspaces_put_sync to check if
 * the most recent dspaces_put is complete or not.
 *
 * Note: ordering of dimension (fast->slow) is 0, 1, ..., n-1. For C row-major
 * array, the dimensions need to be reordered to construct the bounding box. For
 * example, the bounding box for C array c[2][4] is lb: {0,0}, ub: {3,1}. 
 * 
 * @param[in] var_name:     Name of the variable.
 * @param[in] ver:      Version of the variable.
 * @param[in] size:     Size (in bytes) for each element of the global
 *              array.
 * @param[in] ndim:     the number of dimensions for the local bounding
 *              box. 
 * @param[in] lb:       coordinates for the lower corner of the local
 *                  bounding box.
 * @param[in] ub:       coordinates for the upper corner of the local
 *                  bounding box. 
 * @param[in] data:     Pointer to user data buffer. 
 *
 * @return  0 indicates success.
 */
int ndstore_get (ndstore_provider_handle_t provider,
		const char *var_name,
        unsigned int ver, int size,
        int ndim, uint64_t *lb, uint64_t *ub, 
        const void *data); 

#if defined(__cplusplus)
}
#endif

#endif
