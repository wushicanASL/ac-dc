#include "../solver/distributed/distributed_include.h"
#include "../class/Context.h"
#include "../helpers/option_console_parser.h"
#include "../solver/settingsAndStatistics.h"
#include "../utils/file_reader.h"
#include "../solver/Solver.h"
#include "../helpers/utils.h"
#include <math.h>
#include "../utils/distributed_instances_loader.h"
#include "../utils/matrixvector.h"
#include "../solver/distributed/distributed_structures.h"
#include "../helpers/option_distributed_console_parser.h"
#include "ocsidHelper.h"
#include "readWholeData.h"
#include "../solver/distributed/distributed_essentials.h"

//#ifdef MATLAB
//
//#include "class/QuadraticLossLbfgs.h"
//#include "class/LogisticLossMatlab.h"
//
//#endif
#include  <sstream>
int main(int argc, char *argv[]) {
	MPI_Init(&argc, &argv);
	mpi::environment env(argc, argv);
	mpi::communicator world;
	DistributedSettings distributedSettings;
	Context ctx(distributedSettings);
	consoleHelper::parseDistributedOptions(ctx, distributedSettings, argc,
			argv);

	ctx.settings.verbose = true;
	if (world.rank() != 0) {
		ctx.settings.verbose = false;
	}

	ProblemData<unsigned int, double> instance;
	ProblemData<unsigned int, double> newInstance;
	readWholeData(ctx.matrixAFile, instance, false);
	
	unsigned int finalM;
	vall_reduce_maximum(world, &instance.m, &finalM, 1);  //cout << "Local m " << instance.m << "   global m " << finalM << endl;
	instance.m = finalM;   //cout << " Local n " << instance.n << endl;
	vall_reduce(world, &instance.n, &instance.total_n, 1);

	partitionByFeature(instance, newInstance, world.size(), world.rank());
	newInstance.theta = ctx.tmp;
	newInstance.lambda = ctx.lambda;
	newInstance.total_n = instance.total_n;



	double rho = 1.0 / newInstance.n;
	double mu = 0.9;

	std::vector<double> w(newInstance.m);
	//for (unsigned int i = 0; i < instance.m; i++)	w[i] = 0.1*rand() / (RAND_MAX + 0.0);

	std::vector<double> vk(newInstance.m);
	double deltak = 0.0;

	//compute_initial_w(w, instance, rho);
	//for (unsigned int i = 0; i < K; i++){
	//	update_w(w, vk, deltak);
	distributed_PCGByD(w, newInstance, mu, vk, deltak, world, world.size(), world.rank());

	//}
	//MPI::Finalize();

	return 0;

}
