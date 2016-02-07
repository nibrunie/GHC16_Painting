
painting_solver: src/painting_solver.c
	$(CC) $(CFLAGS) $^ -o $@

run: testfiles/learn_and_teach.in painting_solver
	./painting_solver testfiles/learn_and_teach.in
