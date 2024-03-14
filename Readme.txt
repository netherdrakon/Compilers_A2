#To check the execution run the following:

#to create the shared objects in Dataflow folder, execute
make

#then execute following:


opt -bugpoint-enable-legacy-pm=1 -load ./available.so --available ./tests/available-test-m2r.bc -o out
opt -bugpoint-enable-legacy-pm=1 -load ./reaching.so --reaching ./tests/reaching-test-m2r.bc -o out
opt -bugpoint-enable-legacy-pm=1 -load ./available.so --available ./tests/available-test-m2r.bc -o out



