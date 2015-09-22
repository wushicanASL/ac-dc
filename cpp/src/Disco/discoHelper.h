#include <math.h>
#include <gsl/gsl_blas.h> // for BLAS
#include "../solver/distributed/distributed_essentials.h"
#include "QR_solver.h"

#ifndef DISCOHELPER_H_
#define DISCOHELPER_H_

void compute_initial_w(std::vector<double> &w, ProblemData<unsigned int, double> &instance, double &rho) {

}

void update_w(std::vector<double> &w, std::vector<double> &vk, double &deltak) {

}

void compute_objective(std::vector<double> &w, ProblemData<unsigned int, double> &instance, double &obj) {

	obj = 0.0;

	for (unsigned int idx = 0; idx < instance.n; idx++) {

		double w_x = 0.0;
		for (unsigned int i = instance.A_csr_row_ptr[idx]; i < instance.A_csr_row_ptr[idx + 1]; i++)
			w_x += w[instance.A_csr_col_idx[i]] * instance.A_csr_values[i];

		obj += log(1.0 + exp(-1.0 * instance.b[idx] * w_x));
	}

	obj = 1.0 / instance.n * obj + 0.5 * instance.lambda * cblas_l2_norm(w.size(), &w[0], 1);

}

void compute_gradient(std::vector<double> &w, std::vector<double> &gradient,
		ProblemData<unsigned int, double> &instance) {

	double temp = 0.0;
	for (unsigned int idx = 0; idx < instance.n; idx++) {
		double w_x = 0.0;
		for (unsigned int i = instance.A_csr_row_ptr[idx]; i < instance.A_csr_row_ptr[idx + 1]; i++)
			w_x += w[instance.A_csr_col_idx[i]] * instance.A_csr_values[i];

		temp = exp(-1.0 * instance.b[idx] * w_x);
		temp = temp / (1.0 + temp) * (-instance.b[idx]) / instance.total_n;

		for (unsigned int i = instance.A_csr_row_ptr[idx]; i < instance.A_csr_row_ptr[idx + 1]; i++)
			gradient[i] += temp * instance.A_csr_values[i];

	}

	cblas_daxpy(instance.m, instance.lambda, &w[0], 1, &gradient[0], 1);

}

void computeHessianTimesU(std::vector<double> &w, std::vector<double> &u, std::vector<double> &Hu,
		ProblemData<unsigned int, double> &instance, boost::mpi::communicator &world) {

	double temp = 0.0;
	std::vector<double> Hessian(instance.m * instance.m);
	for (unsigned int idx = 0; idx < instance.n; idx++) {
		double w_x = 0.0;
		for (unsigned int i = instance.A_csr_row_ptr[idx]; i < instance.A_csr_row_ptr[idx + 1]; i++)
			w_x += w[instance.A_csr_col_idx[i]] * instance.A_csr_values[i];

		temp = exp(-1.0 * instance.b[idx] * w_x);
		temp = temp / (1.0 + temp) / (1.0 + temp);

		for (unsigned int i = instance.A_csr_row_ptr[idx]; i < instance.A_csr_row_ptr[idx + 1]; i++)
			for (unsigned int j = instance.A_csr_row_ptr[idx]; j < instance.A_csr_row_ptr[idx + 1]; j++)
				Hessian[instance.m * i + j] += temp * instance.A_csr_values[i] * instance.A_csr_values[j]
						/ instance.total_n;

		for (unsigned int i = 0; i < instance.m; i++)
			Hessian[instance.m * i + i] += instance.lambda;
	}

	cblas_dgemv(CblasRowMajor, CblasNoTrans, instance.m, instance.m, 1.0, &Hessian[0], instance.m, &u[0], 1, 1.0,
			&Hu[0], 1);

}

void distributed_PCG(std::vector<double> &w, ProblemData<unsigned int, double> &instance, double &mu,
		std::vector<double> &vk, double &deltak, boost::mpi::communicator &world) {

	// Compute Matrix P
	// Broadcastw_k
	unsigned int K = 100;
	std::vector<double> flag(1);

	// /cout<<world.rank()<<"  "<<w[0]<<endl;

	double epsilon = 1e-5;
	double alpha = 0.0;
	double beta = 0.0;
	instance.m = 100;
//	std::vector<double> P(instance.m * instance.m);
//	std::vector<double> Hessian(instance.m * instance.m);
//	std::vector<double> v(instance.m);
//	std::vector<double> s(instance.m);
//	std::vector<double> r(instance.m);
//	std::vector<double> u(instance.m);
//	std::vector<double> Hu_local(instance.m);
//	std::vector<double> Hu(instance.m);
//	std::vector<double> Hv(instance.m);
//	std::vector<double> gradient(instance.m);
//	std::vector<double> local_gradient(instance.m);
	//cblas_dcopy(instance.m, &r[0], 1, &s[0], 1);
	//cblas_dscal(instance.m, 1.0/mu, &s[0], 1);

	for (unsigned int iter = 0; iter < 10; iter++) {
		// Compute local first derivative
		flag[0] = 1;

//		cblas_set_to_zero(local_gradient);
//		cblas_set_to_zero(Hessian);
//		cblas_set_to_zero(v);
//		cblas_set_to_zero(Hv);
//		vbroadcast(world, w, 0);
//		cout<<iter<<endl;
//		compute_gradient(w, local_gradient, instance);
//		// Aggregates to form f'(w_k)
//		vall_reduce(world, local_gradient, gradient);
//		cblas_dscal(instance.m, 1.0/world.size(), &gradient[0], 1);
//		cblas_dcopy(instance.m, &gradient[0], 1, &r[0], 1);

		if (world.rank() == 0) {
//			double temp = 0.0;
//			for (unsigned int idx = 0; idx < instance.n; idx++){
//				double w_x = 0.0;
//				for (unsigned int i = instance.A_csr_row_ptr[idx];	i < instance.A_csr_row_ptr[idx + 1]; i++)
//					w_x += w[instance.A_csr_col_idx[i]] * instance.A_csr_values[i];
//
//				temp = exp( -1.0 * instance.b[idx] * w_x);
//				temp = temp * (1.0 - temp) / (1.0 + temp) / (1.0 + temp);
//
//				for (unsigned int i = instance.A_csr_row_ptr[idx];	i < instance.A_csr_row_ptr[idx + 1]; i++)
//					for (unsigned int j = instance.A_csr_row_ptr[idx];	j < instance.A_csr_row_ptr[idx + 1]; j++)
//						Hessian[instance.m * i + j] += temp * instance.A_csr_values[i] * instance.A_csr_values[j]
//																											   / instance.total_n;
//			}
//			for (unsigned int i = 0; i < instance.m; i++){
//				Hessian[instance.m * i + i] += instance.lambda;
//				P[instance.m * i + i] = Hessian[instance.m * i + i]+ mu;
//			}
//			// s= p^-1 r
//			CGSolver(P, instance.m, r, s);
//			cblas_dcopy(instance.m, &s[0], 1, &u[0], 1);

		}
		while (flag[0] != 0) {
//			vbroadcast(world, u, 0);
//			cblas_set_to_zero(Hu_local);
//			computeHessianTimesU(w, u, Hu_local, instance, world);
//			vall_reduce(world, Hu_local, Hu);
//			cblas_dscal(instance.m, 1.0/world.size(), &Hu[0], 1);

			if (world.rank() == 0) {

//				//cout<<"I will do this induvidually!!!!!!!!!!"<<endl;
//				double nom = cblas_ddot(instance.m, &r[0], 1, &s[0], 1);
//				double denom = cblas_ddot(instance.m, &u[0], 1, &Hu[0], 1);
//				alpha = nom / denom;
//
//				cblas_daxpy(instance.m, alpha, &u[0], 1, &v[0], 1);
//				cblas_daxpy(instance.m, alpha, &Hu[0], 1, &Hv[0], 1);
//				cblas_daxpy(instance.m, -alpha, &Hu[0], 1, &r[0], 1);
//
//				// ? solve linear system to get new s
//				CGSolver(P, instance.m, r, s);
//
//
//				double nom_new = cblas_ddot(instance.m, &r[0], 1, &s[0], 1);
//				beta = nom_new / nom;
//				cblas_dscal(instance.m, beta, &u[0], 1);
//				cblas_daxpy(instance.m, 1.0, &s[0], 1, &u[0], 1);
//
//				double r_norm = cblas_l2_norm(instance.m, &r[0], 1);
//				cout<<r_norm<<endl;
//
//				// std::vector<double> left(instance.m);
//				// cblas_dgemv(CblasRowMajor, CblasNoTrans, instance.m, instance.m, 1.0, &Hessian[0],
//				// 		 instance.m, &v[0], 1, 1.0, &left[0], 1);
//				// 		cblas_daxpy(instance.m, -1.0, &gradient[0], 1, &left[0], 1);
//				// double error = cblas_l2_norm(instance.m, &left[0], 1);
//				// cout<<error<<endl;
//
//				if (r_norm <= epsilon){
//					cblas_dcopy(instance.m, &v[0], 1, &vk[0], 1);
//					double vHv = cblas_ddot(instance.m, &v[0], 1, &Hv[0], 1); //vHvT^(t) or vHvT^(t+1)
//					double vHu = cblas_ddot(instance.m, &v[0], 1, &Hu[0], 1);
//					deltak = sqrt(vHv + alpha * vHu);
				flag[0] = 0;

				for (int k = 1; k < world.size(); k++) {
//					world.send(k, 0, &flag[0], flag.size());
//					world.send(k, 0, flag);
					vsend(world, k, 0, flag);
				}

//					world.send(1, 0, flag);
//					world.send(2, 0, flag);
//					world.send(3, 0, flag);
//				}

				cout << world.rank() << "    " << flag[0] << endl;
			} else {
				world.recv(0, 0, &flag[0],flag.size() );
//				vrecv(world, 0, 0, flag);
//				world.recv(0, 0, flag);
				cout << world.rank() << "    " << flag[0] << endl;
			}

		}
//		cblas_daxpy(instance.m, 1.0 / (1.0 + deltak), &v[0], 1, &w[0], 1);

	}

}

#endif /* DISCOHELPER_H_ */
