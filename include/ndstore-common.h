/*
 * Copyright (c) 2020, Rutgers Discovery Informatics Institute, Rutgers University
 *
 * See COPYRIGHT in top-level directory.
 */

#ifndef __NDSTORE_COMMON_H
#define __NDSTORE_COMMON_H

#if defined(__cplusplus)
extern "C" {
#endif

#define NDSTORE_SUCCESS          0 /* Success */
#define NDSTORE_ERR_ALLOCATION  -1 /* Error allocating something */
#define NDSTORE_ERR_INVALID_ARG -2 /* An argument is invalid */
#define NDSTORE_ERR_MERCURY     -3 /* An error happened calling a Mercury function */
#define NDSTORE_ERR_PUT         -4 /* Could not put into the server */
#define NDSTORE_ERR_SIZE        -5 /* Client did not allocate enough for the requested data */
#define NDSTORE_ERR_ARGOBOTS    -6 /* Argobots related error */
#define NDSTORE_ERR_UNKNOWN_PR    -7 /* Could not find server */
#define NDSTORE_ERR_UNKNOWN_OBJ    -8 /* Could not find the object*/
#define NDSTORE_ERR_END         -9 /* End of range for valid error codes */


#if defined(__cplusplus)
}
#endif


#endif
