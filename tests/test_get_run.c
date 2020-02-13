
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <margo.h>
#include <ndstore-client.h>
#include "timer.h"
#include "mpi.h"

//# of processors in fortran direction
static int np[10] = {0};
//block size per processor per direction
static uint64_t sp[10] = {0};
//# of interations
static int timesteps_;
//# of processors in the application
static int npapp_;

static int rank_, nproc_;

static uint64_t off[10] = {0};

static struct timer timer_;

static MPI_Comm gcomm_;

static size_t elem_size_;

static char transport_type_str_[256];

static double* allocate_nd(int dims)
{
        double* tmp = NULL;
        int i = 0;
        uint64_t size = 1;
        for(i = 0; i < dims; i++){
                size *= sp[i];
        }
        tmp = (double*)malloc(elem_size_ * size);
        return tmp;
}

static void set_offset_nd(int rank, int dims)
{
	int i = 0, j = 0;
	for(i = 0; i < dims; i++){
		int tmp = rank;
		for(j = 0; j < i; j++)
			tmp /= np[j];
		off[i] = tmp % np[i] * sp[i];
	}
}

void check_data(const char *var_name, double *buf, int num_elem, int rank, int ts)
{
        double max, min, sum, avg;
        int i;
        int cnt = 0;

        if (num_elem <= 0) {
                return;
        }
        max = min = sum = buf[0];
        for (i = 1; i < num_elem; i++) {
                if (max < buf[i])
                        max = buf[i];
                if (min > buf [i])
                        min = buf[i];
                sum += buf[i];
                if (buf[i] != ts) {
                        cnt++;
                }
        }
        avg = sum / num_elem;
        if (cnt > 0) {
                fprintf(stderr, "%s(): var= %s, rank= %d, ts= %d, "
                "error elem cnt= %d, total elem= %d\n",
                        __func__, var_name, rank, ts, cnt, num_elem);
        }

        return;
}

static int couple_read_nd(ndstore_provider_handle_t ndph, unsigned int ts, int num_vars, int dims)
{
	double **data_tab = (double **)malloc(sizeof(double*) * num_vars);
	char var_name[128];
	int i;
	for(i = 0; i < num_vars; i++){
		data_tab[i] = NULL;
	}	

	set_offset_nd(rank_, dims);
	uint64_t dims_size = 1;
	int elem_size = elem_size_;
	uint64_t lb[10] = {0}, ub[10] = {0};
	for(i = 0; i < dims; i++){
		lb[i] = off[i];
		ub[i] = off[i] + sp[i] - 1;
		dims_size *= sp[i];
	}
	double tm_st, tm_end, tm_max, tm_diff;
	int root = 0;


	//allocate data
	double *data = NULL;
	for(i = 0; i < num_vars; i++){
		data = allocate_nd(dims);
		if(data == NULL){
			fprintf(stderr, "%s(): allocate_2d() failed.\n", __func__);
            		return -1; // TODO: free buffers
		}
		memset(data, 0, elem_size_ * dims_size);
		data_tab[i] = data;
	}

	MPI_Barrier(gcomm_);
    tm_st = timer_read(&timer_);
    int err = 0;

	for(i = 0; i < num_vars; i++){
		sprintf(var_name, "mnd_%d", i);
		err = ndstore_get(ndph, var_name, ts, elem_size, dims, lb, ub,
			data_tab[i]);
		if(err!=0){
			fprintf(stderr, "ndstore_get() returned error %d\n", err);
			return err;
		}
		
	}
	tm_end = timer_read(&timer_);
		
	tm_diff = tm_end-tm_st;
	MPI_Reduce(&tm_diff, &tm_max, 1, MPI_DOUBLE, MPI_MAX, root, gcomm_);

    if (rank_ == root) {
        fprintf(stdout, "TS= %u read MAX time= %lf\n",
                ts, tm_max);
    }

	for (i = 0; i < num_vars; i++) {
		sprintf(var_name, "mnd_%d", i);
		check_data(var_name, data_tab[i],dims_size*elem_size_/sizeof(double),
			rank_, ts);
        if (data_tab[i]) {
            free(data_tab[i]);
        }
    }
    free(data_tab);

    return 0;
}

int test_get_run(char *server_addr_str, char *client_addr_prefix, int npapp, int ndims, int* npdim, 
	uint64_t *spdim, int timestep, size_t elem_size, int num_vars, 
	MPI_Comm gcomm)
{
	gcomm_ = gcomm;
	elem_size_ = elem_size;
	timesteps_ = timestep;
	npapp_ = npapp;
	margo_instance_id mid = MARGO_INSTANCE_NULL;

	hg_addr_t svr_addr = HG_ADDR_NULL;
	ndstore_client_t ndcl = NDSTORE_CLIENT_NULL;
	ndstore_provider_handle_t ndph= NDSTORE_PROVIDER_HANDLE_NULL;
	hg_return_t hret = HG_SUCCESS;
    int ret = 0;

	int i;
	for(i = 0; i < ndims; i++){
        np[i] = npdim[i];
        sp[i] = spdim[i];
	}

	timer_init(&timer_, 1);
    timer_start(&timer_);

	double tm_st, tm_end;
	tm_st = timer_read(&timer_);
	
	mid = margo_init(client_addr_prefix, MARGO_CLIENT_MODE, 0, 0);
    if(mid == MARGO_INSTANCE_NULL) {
        fprintf(stderr, "ERROR: margo_initialize()\n");
        ret = -1;
        goto error;
    }

    ret = ndstore_client_init(mid, &ndcl);
    if(ret != NDSTORE_SUCCESS) {
        fprintf(stderr, "ERROR: ndstore_client_init() returned %d\n", ret);
        ret = -1;
        goto error;
    }
    
    margo_addr_lookup(mid, server_addr_str, &svr_addr);
    if(hret != HG_SUCCESS)
    {
        fprintf(stderr, "Error: margo_addr_lookup()\n");
        ndstore_client_finalize(ndcl);
        margo_finalize(mid);
        return(-1);
    }

    ret = ndstore_provider_handle_create(ndcl, svr_addr, 1, &ndph);
    if(ret != NDSTORE_SUCCESS) {
        fprintf(stderr, "ERROR: ndstore_provider_handle_create() returned %d\n", ret);
        ret = -1;
        goto error;
    }

	tm_end = timer_read(&timer_);
	fprintf(stdout, "TIMING_PERF Init_server_connection peer %d time= %lf\n", rank_, tm_end-tm_st);

	MPI_Comm_rank(gcomm_, &rank_);
    MPI_Comm_size(gcomm_, &nproc_);

	unsigned int ts;
	for(ts = 1; ts <= timesteps_; ts++){
		ret = couple_read_nd(ndph, ts, num_vars, ndims);
		if(ret!=0){
			ret = -1;
			goto error;
		}

	}
	
	MPI_Barrier(gcomm_);

	if(rank_ == 0){
		fprintf(stdout, "%s(): done\n", __func__);
	}
	tm_st = timer_read(&timer_);
	ndstore_provider_handle_release(ndph);

    ndstore_client_finalize(ndcl);

    margo_addr_free(mid, svr_addr);

    margo_finalize(mid);
	tm_end = timer_read(&timer_);

	fprintf(stdout, "TIMING_PERF Close_server_connection peer %d time= %lf\n", rank_, tm_end-tm_st);

    return 0;

 error:
 	ndstore_provider_handle_release(ndph);

    ndstore_client_finalize(ndcl);

    margo_addr_free(mid, svr_addr);

    margo_finalize(mid);
    return ret;
    
}
