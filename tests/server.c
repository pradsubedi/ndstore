#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <margo.h>
#include <ndstore-server.h>


int main(int argc, char** argv)
{
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <listen-address>\n", argv[0]);
        return -1;
    }

    hg_return_t hret          = HG_SUCCESS;
    int ret                   = 0;
    char* listen_addr_str     = argv[1];
    uint16_t provider_id      = 1;
    margo_instance_id mid     = MARGO_INSTANCE_NULL;
    ndstore_provider_t ndstore_prov = NDSTORE_PROVIDER_NULL;
    hg_addr_t my_addr         = HG_ADDR_NULL;

    // initialize margo
    mid = margo_init(listen_addr_str, MARGO_SERVER_MODE, 0, -1);
    if(mid == MARGO_INSTANCE_NULL) {
        return -1;
    }

    // get the address we are listening on
    char my_addr_str[256] = {0};
    size_t my_addr_size = 256;
    hret = margo_addr_self(mid, &my_addr);
    if(hret != HG_SUCCESS) {
        fprintf(stderr, "ERROR: margo_addr_self() returned %d\n", hret);
        ret = -1;
        goto error;
    }
    hret = margo_addr_to_string(mid, my_addr_str, &my_addr_size, my_addr);
    if(hret != HG_SUCCESS) {
        fprintf(stderr, "ERROR: margo_addr_to_string() returned %d\n", hret);
        ret = -1;
        margo_addr_free(mid, my_addr);
        goto error;
    }
    margo_addr_free(mid, my_addr);
    fprintf(stdout,"Server running at address %s\n", my_addr_str);

    // create the NDSTORE provider
    ret = ndstore_provider_register(mid, 1, NDSTORE_ABT_POOL_DEFAULT, &ndstore_prov);
    if(ret != NDSTORE_SUCCESS) {
        fprintf(stderr, "ERROR: ndstore_provider_register() returned %d\n", ret);
        ret = -1;
        goto error;
    }


    // make margo wait for finalize
    margo_wait_for_finalize(mid);

finish:
    return ret;
error:
    margo_finalize(mid);
    goto finish;
}
