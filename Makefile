target = /cfs/klemming/scratch/j/jacobwah/heat

all: $(target)

$(target): heat.c
	cc -O2 -o $@ $^
