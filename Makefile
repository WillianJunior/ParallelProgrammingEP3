# $(VERBOSE).SILENT:

CXXC = mpicxx
CFLAGS = -std=c++11 -lm -g -ggdb -gdwarf-2
SERIAL = ep3-serial
CONC = ep3-conc

HUGE = 5000

all: compile

compile: $(SERIAL).cpp
	$(CXXC) $(CFLAGS) $(SERIAL).cpp -o $(SERIAL).out
	$(CXXC) $(CFLAGS) $(CONC).cpp -o $(CONC).out

test: test_inputs_gen test_outputs_gen compile
	@./$(SERIAL).out 3 3 1 1 ocean < test1 > r
	@-diff r test1_r
	@./$(SERIAL).out 4 4 2 4 ocean < test2 > r
	@-diff r test2_r
	@./$(SERIAL).out 10 10 5 2 ocean < test3 > r
	@-diff r test3_r

	@mpirun -np $NP $(CONC).out 3 3 1 1 ocean < test1 > r
	@-diff r test1_r
	@mpirun -np $NP $(CONC).out 4 4 2 4 ocean < test2 > r
	@-diff r test2_r
	@mpirun -np $NP $(CONC).out 10 10 5 2 ocean < test3 > r
	@-diff r test3_r

test_huge: compile test_huge_input_gen
	@ echo "serial: "
	@./$(SERIAL).out $(HUGE) $(HUGE) 5 2 time < test_huge
	@ echo "concurrent: "
	@./$(CONC).out $(HUGE) $(HUGE) 5 2 time < test_huge


test_inputs_gen:
	@ printf "5\n5\n5\n5\n5\n9\n5\n5\n5\n" > test1
	@ printf "2\n2\n2\n2\n1\n2\n3\n4\n1\n2\n2\n1\n3\n3\n3\n3\n" > test2
	@ printf "5\n5\n5\n5\n5\n5\n5\n5\n5\n5\n" > test3
	@ printf "5\n5\n5\n5\n5\n5\n5\n5\n5\n5\n" >> test3
	@ printf "5\n5\n5\n5\n5\n5\n5\n5\n5\n5\n" >> test3
	@ printf "5\n5\n5\n5\n5\n5\n5\n5\n5\n5\n" >> test3
	@ printf "5\n5\n5\n5\n5\n5\n5\n5\n5\n5\n" >> test3
	@ printf "5\n5\n5\n5\n5\n11\n5\n5\n5\n5\n" >> test3
	@ printf "5\n5\n5\n5\n5\n5\n5\n5\n5\n5\n" >> test3
	@ printf "5\n5\n5\n5\n5\n5\n5\n5\n5\n5\n" >> test3
	@ printf "5\n5\n5\n5\n5\n5\n5\n5\n5\n5\n" >> test3
	@ printf "5\n5\n5\n5\n5\n5\n5\n5\n5\n5\n" >> test3

test_outputs_gen:
	@ printf "4\t8\t8\n4\t8\t8\n4\t8\t8\n" > test1_r
	@ printf "1\t2\t2\t2\n1\t2\t2\t2\n1\t2\t2\t2\n1\t2\t2\t2\n" > test2_r
	@ printf "6\t6\t6\t6\t6\t6\t6\t6\t6\t6\n" > test3_r
	@ printf "6\t6\t6\t6\t6\t6\t6\t6\t6\t6\n" >> test3_r
	@ printf "6\t6\t6\t6\t6\t6\t6\t6\t6\t6\n" >> test3_r
	@ printf "6\t6\t6\t6\t6\t6\t6\t6\t6\t6\n" >> test3_r
	@ printf "6\t6\t6\t6\t6\t6\t6\t6\t6\t6\n" >> test3_r
	@ printf "6\t6\t6\t6\t6\t6\t6\t6\t6\t6\n" >> test3_r
	@ printf "6\t6\t6\t6\t6\t6\t6\t6\t6\t6\n" >> test3_r
	@ printf "6\t6\t6\t6\t6\t6\t6\t6\t6\t6\n" >> test3_r
	@ printf "6\t6\t6\t6\t6\t6\t6\t6\t6\t6\n" >> test3_r
	@ printf "6\t6\t6\t6\t6\t6\t6\t6\t6\t6\n" >> test3_r

test_huge_input_gen:
	@ perl huge_gen.pl $(HUGE)