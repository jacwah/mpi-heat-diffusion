# Heat diffusion solver with MPI

This is an implementation of the final project of the course DD2356 High
Performance Computing in the spring 2021. The program `heat.c` is a solver for
the heat equation in 2D on a rectangular domain with constant boundary
conditions. It is parallelized using MPI. The solution is written to a number of
binary files, and can be visualized using the `vis.py` script.

## MPI structure

Each process owns a square tile of the domain.
They use a Cartesian communicator with a neighborhood collective. The
halo exchange is implemented as a single call to `MPI_Neighbor_alltoall`,
preceeded by copies to a send buffer, and followed by copies from a receive
buffer.

## Output files

One process writes the parameters for domain size, number of timesteps and
number of processes to the file `heat-meta.bin`. After each timestep, the
processes write their tile to the file `heat-x-y.bin` where `x` and `y` give the
coordinate of the tile.

After the execution, the files can be transfered to a local computer and
visualized with the `vis.py` script.
