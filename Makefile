target = /cfs/klemming/scratch/j/jacobwah/heat

all: $(target)

$(target): heat.c
	cc -o $@ $^

srun: $(target)
	cd $(dir $(target)); srun -n8 ./$(notdir $(target))
