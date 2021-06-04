#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define T_INITIAL 0.0
#define T_BOUNDARY 100.0

#define RDTSC_START()            \
	__asm__ volatile("CPUID\n\t" \
	                 "RDTSC\n\t" \
	                 "mov %%edx, %0\n\t" \
	                 "mov %%eax, %1\n\t" \
	                 : "=r" (start_hi), "=r" (start_lo) \
	                 :: "%rax", "%rbx", "%rcx", "%rdx");

#define RDTSC_STOP()              \
	__asm__ volatile("RDTSCP\n\t" \
	                 "mov %%edx, %0\n\t" \
	                 "mov %%eax, %1\n\t" \
	                 "CPUID\n\t" \
	                 : "=r" (end_hi), "=r" (end_lo) \
	                 :: "%rax", "%rbx", "%rcx", "%rdx");

void xerror(const char *s)
{
	if (errno) {
		fprintf(stderr, "errno=%d\n", errno);
		perror(s);
		exit(1);
	}
}

int main(int argc, char **argv)
{
	int provided;
	MPI_Init_thread(&argc, &argv, MPI_THREAD_SINGLE, &provided);

	double t0 = MPI_Wtime();

	assert(argc == 4);

	long X = atol(argv[1]), Y = atol(argv[2]);
	int T = atoi(argv[3]);

	int nprocs, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if ((X*Y) % nprocs != 0) {
		if (rank == 0)
			fprintf(stderr, "Domain %ld,%ld not compatible with %d processes\n", X, Y, nprocs);
		exit(1);
	}

	int n = sqrt((X*Y) / nprocs);

	assert(n*n == (X*Y) / nprocs);

	double D = 4.0;

	double dx = 1.0 / X;
	double dy = 1.0 / Y;

	double dx2 = dx*dx;
	double dy2 = dy*dy;
	double dt = dx2 * dy2 / (2.0 * D * (dx2 + dy2));

	int dims[] = {X/n, Y/n};
	assert((X/n)*(Y/n) == nprocs);

	int periods[] = {0, 0};
	MPI_Comm comm_cart;
	MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &comm_cart);
	MPI_Comm_rank(comm_cart, &rank);

	int coords[2];
	MPI_Cart_coords(comm_cart, rank, 2, coords);

	int rank_down, rank_up;
	MPI_Cart_shift(comm_cart, 0, 1, &rank_down, &rank_up);
	int rank_left, rank_right;
	MPI_Cart_shift(comm_cart, 1, 1, &rank_left, &rank_right);

	double (*u0)[n+2] = malloc(sizeof(double)*(n+2)*(n+2));
	double (*u)[n+2] = malloc(sizeof(double)*(n+2)*(n+2));

	double *sendbuf = malloc(sizeof(double)*n*4);
	double *recvbuf = malloc(sizeof(double)*n*4);

	errno = 0;

	if (rank == 0) {
		int metafd = open("heat-meta.bin", O_WRONLY|O_CREAT|O_TRUNC, 0600);
		xerror("meta open");
#ifdef HEAT_WRITE_ALL
		int meta[] = {X, Y, T, nprocs};
#else
		int meta[] = {X, Y, 1, nprocs};
#endif
		write(metafd, meta, sizeof(meta));
		close(metafd);
	}

	char fname[32] = {};
	sprintf(fname, "heat-%d-%d.bin", coords[0], coords[1]);
	int tilefd = open(fname, O_WRONLY|O_CREAT|O_TRUNC, 0600);

	xerror("tile open");

	for (int i = 1; i <= n; i++)
		for (int j = 1; j <= n; j++)
			u0[i][j] = T_INITIAL;

	// If there is no sending neighbor, there must be a boundary
	for (int i = 0; i < n*4; i++)
		recvbuf[i] = T_BOUNDARY;

#ifdef HEAT_IDLE
	uint64_t *idle_cycles = malloc(sizeof(uint64_t)*T*2);

	uint32_t start_hi, start_lo, end_hi, end_lo;
	RDTSC_START();
	RDTSC_STOP();
	uint64_t cycle0 = ((((uint64_t)end_hi) << 32) | end_lo);
#endif

	for (int t = 0; t < T; t++) {
		// Order: down, up, left, right
		for (int i = 1; i <= n; i++)
			sendbuf[0*n + i-1] = u0[1][i];
		for (int i = 1; i <= n; i++)
			sendbuf[1*n + i-1] = u0[n][i];
		for (int j = 1; j <= n; j++)
			sendbuf[2*n + j-1] = u0[j][1];
		for (int j = 1; j <= n; j++)
			sendbuf[3*n + j-1] = u0[j][n];

#ifdef HEAT_IDLE
		MPI_Request req;
		MPI_Ineighbor_alltoall(sendbuf, n, MPI_DOUBLE, recvbuf, n, MPI_DOUBLE, comm_cart, &req);
		RDTSC_START();
		MPI_Wait(&req, MPI_STATUS_IGNORE);
		RDTSC_STOP();

		idle_cycles[2*t+0] = ((((uint64_t)end_hi) << 32) | end_lo) - cycle0;
		idle_cycles[2*t+1] = ((((uint64_t)start_hi) << 32) | start_lo) - cycle0;
#else
		MPI_Neighbor_alltoall(sendbuf, n, MPI_DOUBLE, recvbuf, n, MPI_DOUBLE, comm_cart);
#endif

		for (int i = 1; i <= n; i++)
			u0[0][i] = recvbuf[0*n + i-1];
		for (int i = 1; i <= n; i++)
			u0[n+1][i] = recvbuf[1*n + i-1];
		for (int j = 1; j <= n; j++)
			u0[j][0] = recvbuf[2*n + j-1];
		for (int j = 1; j <= n; j++)
			u0[j][n+1] = recvbuf[3*n + j-1];

		for (int i = 1; i <= n; i++) {
			for (int j = 1; j <= n; j++) {
				double uxx = (u0[i+1][j] - 2*u0[i][j] + u0[i-1][j]) / dx2;
				double uyy = (u0[i][j+1] - 2*u0[i][j] + u0[i][j-1]) / dy2;
				u[i][j] = u0[i][j] + dt * D * (uxx + uyy);
			}
		}

#ifdef HEAT_WRITE_ALL
		errno = 0;
		for (int j = 1; j <= n; j++) {
			write(tilefd, &u[j][1], sizeof(double)*n);
			xerror("tile write");
		}
#endif

		void *tmp = u0;
		u0 = u;
		u = tmp;
	}

	if (rank == 0) {
		double t1 = MPI_Wtime();
		printf("%e\n", t1-t0);
	}

#ifndef HEAT_WRITE_ALL
		errno = 0;
		for (int j = 1; j <= n; j++) {
			write(tilefd, &u[j][1], sizeof(double)*n);
			xerror("tile write");
		}
#endif

#ifdef HEAT_IDLE
	sprintf(fname, "idle-%d-%d.bin", coords[0], coords[1]);
	int idlefd = open(fname, O_WRONLY|O_CREAT|O_TRUNC, 0600);
	xerror("idle open");
	write(idlefd, idle_cycles, sizeof(uint64_t)*T*2);
	close(idlefd);
#endif
}
