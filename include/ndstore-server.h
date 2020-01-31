/*
 * Copyright (c) 2020, Rutgers Discovery Informatics Institute, Rutgers University
 *
 * See COPYRIGHT in top-level directory.
 */


#ifndef __NDSTORE_SERVER_H
#define __NDSTORE_SERVER_H

#include <margo.h>
#include <ndstore-common.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define NDSTORE_ABT_POOL_DEFAULT ABT_POOL_NULL

typedef struct ndstore_provider* ndstore_provider_t;
#define NDSTORE_PROVIDER_NULL ((ndstore_provider_t)NULL)
#define NDSTORE_PROVIDER_IGNORE ((ndstore_provider_t*)NULL)

/**
 * @brief Creates a new NDSTORE provider. If NDSTORE_PROVIDER_IGNORE
 * is passed as last argument, the provider will be automatically
 * destroyed when calling :code:`margo_finalize`.
 *
 * @param[in] mid Margo instance
 * @param[in] provider_id provider id
 * @param[in] pool Argobots pool
 * @param[out] provider provider handle
 *
 * @return NDSTORE_SUCCESS or error code defined in ndstore-common.h
 */
int ndstore_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        ABT_pool pool,
        ndstore_provider_t* provider);

/**
 * @brief Destroys the Ndstore provider and deregisters its RPC.
 *
 * @param[in] provider Ndstore provider
 *
 * @return NDSTORE_SUCCESS or error code defined in ndstore-common.h
 */
int ndstore_provider_destroy(
        ndstore_provider_t provider);

#if defined(__cplusplus)
}
#endif

#endif
