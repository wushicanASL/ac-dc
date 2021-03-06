/*
 * QuadraticLoss.h
 *
 *  Created on: Dec 1, 2014
 *      Author: taki
 */

#ifndef QUADRATICLOSS_H_
#define QUADRATICLOSS_H_


#include "cocoaLossFunction.h"

template<typename L, typename D>
class QuadraticLoss: public LossFunction<L, D>  {
public:
	QuadraticLoss() {

	}

	virtual ~QuadraticLoss() {}

	virtual void computeObjectiveValue(ProblemData<L, D> & instance,
	                                   mpi::communicator & world, std::vector<D> & w, double &finalDualError,
	                                   double &finalPrimalError) {

		D localError = 0.0;
		for (unsigned int i = 0; i < instance.n; i++) {
			D tmp = instance.x[i] * instance.x[i] * 0.5 - instance.b[i] * instance.x[i];
			localError += tmp;
		}

		D localQuadLoss = 0.0;
		for (unsigned int idx = 0; idx < instance.n; idx++) {
			D dotProduct = 0.0;
			for (L i = instance.A_csr_row_ptr[idx];
			        i < instance.A_csr_row_ptr[idx + 1]; i++) {
				dotProduct += (w[instance.A_csr_col_idx[i]])
				              * instance.A_csr_values[i];
			}
			D tmp = 0.5 * (1.0 * instance.b[idx] -  dotProduct * instance.b[idx]) * (1.0 * instance.b[idx] - dotProduct * instance.b[idx]);

			localQuadLoss += tmp;

		}

		finalPrimalError = 0;
		vall_reduce(world, &localQuadLoss, &finalPrimalError, 1);

		finalDualError = 0;
		//cout<<localError<<endl;
		vall_reduce(world, &localError, &finalDualError, 1);
		//cout<<finalDualError<<endl;

		D tmp2 = cblas_l2_norm(w.size(), &w[0], 1);
		finalDualError = 1.0 / (0.0 + instance.total_n) * finalDualError
		                 + 0.5 * instance.lambda * tmp2 * tmp2;
		finalPrimalError =  1.0 / (0.0 + instance.total_n) * finalPrimalError
		                    + 0.5 * instance.lambda * tmp2 * tmp2;
	}


	virtual void compute_subproproblem_obj(ProblemData<L, D> &instance,
	                                       std::vector<D> &potent, std::vector<D> &w, D &potent_obj) {
		D obj_temp1 = 0.0;
		D obj_temp2 = 0.0;
		D obj_temp3 = 0.0;
		for (unsigned int i = 0; i < instance.n; i++) {
			obj_temp1 += 0.5 * (instance.x[i] + potent[i]) * (instance.x[i] + potent[i])
			             - (instance.x[i] + potent[i]) * instance.b[i];
		}

		std::vector<D> Apotent(instance.m);
		cblas_set_to_zero(Apotent);
		vectormatrix_b(potent, instance.A_csr_values, instance.A_csr_col_idx, instance.A_csr_row_ptr,
		               instance.b, 1.0, instance.n, Apotent);

		D g_norm = cblas_l2_norm(instance.m, &Apotent[0], 1);
		obj_temp3 = g_norm * g_norm;

		for (L i = 0; i < instance.m; i++)
			obj_temp2 += w[i] * Apotent[i];

		potent_obj = 1.0 / instance.total_n * (obj_temp1 + obj_temp2 + 0.5 * instance.penalty * instance.oneOverLambdaN * obj_temp3);

	}


	virtual void compute_gradient(std::vector<D> &w, std::vector<D> &grad,
	                              ProblemData<L, D> &instance) {

		cblas_set_to_zero(grad);
		double temp = 0.0;
		double w_x = 0.0;
		// for (unsigned int idx = 0; idx < instance.n; idx++) {
		// 	w_x = 0.0;
		// 	for (unsigned int i = instance.A_csr_row_ptr[idx]; i < instance.A_csr_row_ptr[idx + 1]; i++)
		// 		w_x += w[instance.A_csr_col_idx[i]] * instance.A_csr_values[i];

		// 	temp = exp(-1.0 * instance.b[idx] * w_x);
		// 	temp = temp / (1.0 + temp) * (-instance.b[idx]) / instance.total_n;
		// 	for (unsigned int i = instance.A_csr_row_ptr[idx]; i < instance.A_csr_row_ptr[idx + 1]; i++)
		// 		grad[instance.A_csr_col_idx[i]] += temp * instance.A_csr_values[i];
		// }
		for (unsigned int idx = 0; idx < instance.n; idx++) {
			w_x = 0.0;
			for (unsigned int i = instance.A_csr_row_ptr[idx]; i < instance.A_csr_row_ptr[idx + 1]; i++)
				w_x += w[instance.A_csr_col_idx[i]] * instance.A_csr_values[i];

			temp = (w_x * instance.b[idx] - instance.b[idx]) / instance.n;
			//temp = temp / (1.0 + temp) * (-instance.b[idx]) / instance.total_n;
			for (unsigned int i = instance.A_csr_row_ptr[idx]; i < instance.A_csr_row_ptr[idx + 1]; i++)
				grad[instance.A_csr_col_idx[i]] += temp * instance.A_csr_values[i] * instance.b[idx];
		}

		cblas_daxpy(instance.m, instance.lambda, &w[0], 1, &grad[0], 1);
	}

	virtual void compute_subproproblem_gradient(ProblemData<L, D> &instance,
	        std::vector<D> &gradient, std::vector<D> &deltaAlpha, std::vector<D> &w) {

		std::vector<D> gradient_temp1(instance.n);
		std::vector<D> gradient_temp2(instance.m);
		std::vector<D> gradient_temp3(instance.n);

		matrixvector(instance.A_csr_values, instance.A_csr_col_idx, instance.A_csr_row_ptr, w, instance.n, gradient_temp1);
		vectormatrix_b(deltaAlpha, instance.A_csr_values, instance.A_csr_col_idx, instance.A_csr_row_ptr,
		               instance.b, 1.0, instance.n, gradient_temp2);
		matrixvector(instance.A_csr_values, instance.A_csr_col_idx, instance.A_csr_row_ptr,
		             gradient_temp2, instance.n, gradient_temp3);

		for (L i = 0; i < instance.n; i++) {
			gradient[i] = 1.0 / instance.total_n * ( gradient_temp1[i] * instance.b[i] + instance.x[i] - instance.b[i]
			              + 1.0 * instance.penalty * instance.oneOverLambdaN  * gradient_temp3[i] * instance.b[i] + deltaAlpha[i] );
		}

	}

	virtual void backtrack_linesearch(ProblemData<L, D> &instance,
	                                  std::vector<D> &deltaAlpha, std::vector<D> &search_direction, std::vector<D> &w, D &dualobj,
	                                  D &rho, D &c1ls, D &a, DistributedSettings & distributedSettings) {

		int iter = 0;
		std::vector<D> potent(instance.n);

		while (1) {

			if (iter > 100) {
				//cout<<a<<"    ";
				//a = 1.0;
				break;
			}

			for (L i = 0; i < instance.n; i++) {
				potent[i] = deltaAlpha[i] - a * search_direction[i];
			}

			D obj = 0.0;

			this->compute_subproproblem_obj(instance, potent, w, obj);

			D gg;
			gg = cblas_l2_norm(instance.n, &search_direction[0], 1);
			//cout<<a<<"    "<<obj<<"    "<<dualobj - c1ls * a * gg * gg<<endl;
			if (obj <= dualobj - c1ls * a * gg * gg) {
				//cout<<a<<"    "<<obj<<"  "<<dualobj - c1ls * a * gg * gg<<"    "<<a<<endl;
				//cblas_set_to_zero(deltaW);
				for (L idx = 0; idx < instance.n; idx++) {
					//instance.x[idx] = potent[idx];
					deltaAlpha[idx] = potent[idx]; // - a * gradient[idx];
					//						for (L i = instance.A_csr_row_ptr[idx];	i < instance.A_csr_row_ptr[idx + 1]; i++)
					//							deltaW[instance.A_csr_col_idx[i]] += instance.oneOverLambdaN
					//							* instance.A_csr_values[i] * deltaAlpha[idx]* instance.b[idx];
				}
				//				vectormatrix_b(instance.x, instance.A_csr_values, instance.A_csr_col_idx,instance.A_csr_row_ptr,
				//							 instance.b, instance.oneOverLambdaN, instance.n, deltaW);
				//				cout << iter <<endl;

				dualobj = obj;

				break;
			}
			a = rho * a;
			iter += 1;
		}
	}


	virtual void LBFGS_update(ProblemData<L, D> &instance, std::vector<D> &search_direction, std::vector<D> &old_grad,
	                          std::vector<D> &sk, std::vector<D> &rk, std::vector<D> &gradient, std::vector<D> &oneoversy,
	                          L iter_counter, int limit_BFGS, int flag_BFGS) {

		for (L i = 0; i < instance.n; i++) {

			if (iter_counter > 0) {
				if (flag_BFGS > 0)
					rk[instance.n * (flag_BFGS - 1)  + i] = gradient[i] - old_grad[i];
				else
					rk[instance.n * (limit_BFGS - 1)  + i] = gradient[i] - old_grad[i];
			}

			old_grad[i] = gradient[i];
			search_direction[i] = gradient[i];
		}

		if (iter_counter > 0) {

			int flag_old = flag_BFGS - 1;
			if (flag_BFGS == 0)
				flag_old = limit_BFGS - 1;

			oneoversy[flag_old] = 0;
			for (L idx = 0; idx < instance.n; idx++) {
				oneoversy[flag_old] += rk[instance.n * flag_old + idx] * sk[instance.n * flag_old + idx];
			}
			oneoversy[flag_old] = 1.0 / oneoversy[flag_old];

			L kai = flag_BFGS;
			L wan = kai + limit_BFGS - 1;// min(kai + 9, kai + iter_counter - 1);
			std::vector<D> aa(limit_BFGS);
			cblas_set_to_zero(aa);
			D bb = 0.0;
			//cout<<kai<<"    "<<wan<<"    "<<iter_counter<<endl;

			if (iter_counter < limit_BFGS) {
				kai = 0;
				wan = flag_BFGS - 1;
			}

			for (L i = kai; i <= wan; i++) {
				L ii = wan - (i - kai);
				if (ii >= limit_BFGS)
					ii = ii - 10;

				for (L j = 0; j < instance.n; j++)
					aa[ii] += sk[instance.n * ii + j] * search_direction[j];
				aa[ii] *= oneoversy[ii];
				for (L idx = 0; idx < instance.n; idx++)
					search_direction[idx] -= aa[ii] * rk[instance.n * ii + idx];
			}
			D Hk_zero = 0.0;
			D t1 = 0.0;
			D t2 = 0.0;
			for (L idx = instance.n * flag_old; idx < instance.n * (flag_old + 1); idx++) {
				t1 += sk[idx] * rk[idx];
				t2 += rk[idx] * rk[idx];
				//cout<< t1 << "   "<<t2<<endl;
			}

			Hk_zero = t1 / t2;
			//cout<< Hk_zero << "   "<<t2<<endl;
			for (L idx = 0; idx < instance.n; idx++)
				search_direction[idx] = Hk_zero * search_direction[idx];

			for (L i = kai; i <= wan; i++) {
				L ii = i;
				if (i >= limit_BFGS)
					ii = i - limit_BFGS;

				bb = 0.0;
				for (L j = 0; j < instance.n; j++)
					bb += rk[instance.n * ii + j] * search_direction[j];
				bb *= oneoversy[ii];
				for (L j = 0; j < instance.n; j++)
					search_direction[j] += sk[instance.n * ii + j] * (aa[ii] - bb);
			}

		}
	}


};

#endif /* QUADRATICLOSS_H_ */
