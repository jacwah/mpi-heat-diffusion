# Change this to your own scratch directory
builddir = /cfs/klemming/scratch/j/jacobwah
target = $(builddir)/heat

all: $(target) $(target)-idle $(target)-writeall

$(target)-idle: heat.c
	cc -DHEAT_IDLE -O2 -o $@ $^

$(target)-writeall: heat.c
	cc -DHEAT_WRITE_ALL -O2 -o $@ $^

$(target): heat.c
	cc -O2 -o $@ $^

.PHONY: all
