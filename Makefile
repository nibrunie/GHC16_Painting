
painting_solver: src/painting_solver.c
	$(CC) $(CFLAGS) $^ -o $@

run: painting_solver
	./painting_solver testfiles/learn_and_teach.in
	./painting_solver testfiles/logo.in
	./painting_solver testfiles/right_angle.in
