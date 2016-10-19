#include <iostream>
#include <string>
#include <list>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

#include <mpi.h>

#include <mutex>

using namespace std;

mutex m;

float* ocean1;
float* ocean2;

enum run_t {
	r_ocean, r_time, r_all
};

string usage = "usage: ./ep2 <i_size> <j_size> <u> <n_threads> <ocean|time|all>";

void print_ocean(float* ocean, int i_size, int j_size) {
	for (int i=0; i<i_size; i++) {
		cout << *(ocean1+i*i_size);
		for (int j=1; j<j_size; j++) {
			cout << "\t" << *(ocean1+i*i_size+j);
		}
		cout << endl;
	}
}

float update_ocean_val(int i, int j, int i_size, int j_size) {
	float max = *(ocean1+i*i_size+j);
	if (i != 0){
		if (j != 0)
			if (*(ocean1+(i-1)*i_size+j-1) > max)
				max = *(ocean1+(i-1)*i_size+j-1);

		if (*(ocean1+(i-1)*i_size+j) > max)
			max = *(ocean1+(i-1)*i_size+j);

		if (j != j_size-1)
			if (*(ocean1+(i-1)*i_size+j+1) > max)
				max = *(ocean1+(i-1)*i_size+j+1);
	}

	if (j != 0)
		if (*(ocean1+i*i_size+j-1) > max)
			max = *(ocean1+i*i_size+j-1);
	if (j != j_size-1)
		if (*(ocean1+i*i_size+j+1) > max)
			max = *(ocean1+i*i_size+j+1);

	if (i != i_size-1){
		if (j != 0)
			if (*(ocean1+(i+1)*i_size+j-1) > max)
				max = *(ocean1+(i+1)*i_size+j-1);
		
		if (*(ocean1+(i+1)*i_size+j) > max)
			max = *(ocean1+(i+1)*i_size+j);

		if (j != j_size-1)
			if (*(ocean1+(i+1)*i_size+j+1) > max)
				max = *(ocean1+(i+1)*i_size+j+1);
	}

	return max-1;
}

int main(int argc, char *argv[]) {
	
	// get args
	int i_size, j_size;
	int u;
	int n_threads;
	run_t run_type;
	if (argc != 6) {
		cout << usage << endl;
		return 1;
	}

	i_size = stoi(string(argv[1]), nullptr);
	j_size = stoi(string(argv[2]), nullptr);
	u = stoi(string(argv[3]), nullptr);
	n_threads = stoi(string(argv[4]));

	if (strcmp(argv[5], "ocean") == 0)
		run_type = r_ocean;
	else if (strcmp(argv[5], "time") == 0)
		run_type = r_time;
	else if (strcmp(argv[5], "all") == 0)
		run_type = r_all;
	else {
		std::cout << usage << std::endl;
		return 2;	
	}

	// allocate oceans
	ocean1 = new float[i_size*j_size];
	ocean2 = new float[i_size*j_size];

	// get ocean values
	for (int i=0; i<i_size*j_size; i++) {
		cin >> ocean1[i];
	}

	struct timeval beginT, endT;
	gettimeofday(&beginT,NULL);

	MPI_Init(&argc, &argv);
	int np;
	MPI_Comm_size(MPI_COMM_WORLD, &np);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int lines = ceil(i_size/np);
	int pi = lines*rank-1;
	pi = pi<0 ? 0 : pi;
	int pj = lines*(rank+1);

	// scatter ocean lines across mpi processes
	if (rank == np-1) {
		MPI_Send((void*)ocean1, lines*j_size, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
		for (int i=1; i<np-1; i++)
			MPI_Send((void*)(ocean1+(lines*i-1)*j_size), lines*j_size, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
		pj = i_size-1;
	} else {
		MPI_Recv ((void*)(ocean1+pi*j_size), lines*j_size, MPI_FLOAT, np-1, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		m.lock();
		cout << "Rank " << rank << endl;
		print_ocean(ocean1, i_size, j_size);
		m.unlock();
	}



	// perform u iterations
	for (int uu=0; uu<u; uu++) {
		for (int i=pi+1; i<pj-1; i++) {
			for (int j=0; j<j_size; j++) {
				*(ocean2+i*i_size+j) = update_ocean_val(i, j, i_size, j_size);
			}
		}
		float* temp_ocean = ocean1;
		ocean1 = ocean2;
		ocean2 = temp_ocean;

		// cout << "it " << uu << endl;
		// print_ocean(ocean1, i_size, j_size);
		
		// send update to neighbors
		MPI_Request send_request1;
		MPI_Request send_request2;
		MPI_Isend((void*)(ocean1+(pi+1)*j_size), lines*j_size, MPI_FLOAT, rank-1, 0, MPI_COMM_WORLD, &send_request1);
		MPI_Isend((void*)(ocean1+(pj-1)*j_size), lines*j_size, MPI_FLOAT, rank+1, 0, MPI_COMM_WORLD, &send_request2);

		// get update from neighbors
		MPI_Recv((void*)(ocean1+pi*j_size), lines*j_size, MPI_FLOAT, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv((void*)(ocean1+pj*j_size), lines*j_size, MPI_FLOAT, rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}


	MPI_Finalize();

	gettimeofday(&endT,NULL);

	// print ocean if this was selected
	if (run_type == r_ocean || run_type == r_all) {
		print_ocean(ocean1, i_size, j_size);
	}

	// print time if this was selected
	if (run_type == r_time || run_type == r_all) {
		__time_t time = ((endT.tv_sec*1000000+endT.tv_usec)-
			(beginT.tv_sec*1000000+beginT.tv_usec));

		cout << time << endl;
	}

	return 0;
}