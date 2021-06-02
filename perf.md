To perform strong scalability test we need to run the program with the same
domain size but for different number of processes. We need to fulfill the
following condition.

XY mod N = 0

The smallest square XY to fulfill this for all squares <= 256 is 720720^2
which is way too large. Therefore we need to exclude some process counts. By
excluding 11^2 and 13^2 processes we instead get 5040^2 which is manageable. 
