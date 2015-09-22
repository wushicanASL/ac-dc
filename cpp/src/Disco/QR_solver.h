#include <iostream>
#include <iostream>
#include <math.h>
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <vector>
#include <iomanip>      // std::setprecision
#include <set>
using namespace std;

#include <gsl/gsl_blas.h> // for BLAS
#include <gsl/gsl_linalg.h> // for LU, QR, ...




void gaussEliminationSolver(std::vector<double> &A, int n, 
	std::vector<double> &b, std::vector<double> &x) {

	std::vector<double> A_fix(n * n, 0);
	std::vector<double> b_fix(n, 0);
	cblas_dcopy(n * n, &A[0], 1, &A_fix[0], 1);
	cblas_dcopy(n, &b[0], 1, &b_fix[0], 1);

	for (int col_idx = 0; col_idx < n - 1; col_idx++){
		for (int row_idx = col_idx + 1; row_idx < n; row_idx++){
			int idx1 = row_idx * n + col_idx;
			int idx2 = col_idx * n + col_idx;
			double multiplier = 0.0;
			multiplier = 1.0 * A_fix[idx1] / A_fix[idx2];
			//http://www.mathkeisan.com/usersguide/man/daxpy.html
			int idx_start1 = row_idx * n + col_idx;
			int idx_start2 = col_idx * n + col_idx;
			cblas_daxpy(n - col_idx, -multiplier, &A_fix[idx_start2], 1, &A_fix[idx_start1], 1);
			b_fix[row_idx] -= 1.0 * multiplier * b_fix[col_idx];
		}
	}

	for (int i = n - 1; i >= 0; i--){
		int idx = i * n + i + 1;

		double dot_product = cblas_ddot(n - i - 1, &A_fix[idx], 1, &x[i+1], 1);
		x[i] = (b_fix[i] - dot_product) / A_fix[idx - 1];
	}

}



void QRGramSchmidtSolver(std::vector<double> &A, int n, 
	std::vector<double> &b, std::vector<double> &x) {

	// Generating Q^T
	std::vector<double> QT(n * n, 0);
	// Copy A^T to Q^T
	for (int i = 0; i < n; i++)
		cblas_dcopy(n, &A[i], n, &QT[i*n],1);
	
	// Compute Q
	for  (int i = 1; i < n; i++){
		double denom = cblas_dnrm2(n, &QT[(i-1)*n], 1);
		for (int j = i; j < n; j++){
			double nom = cblas_ddot(n, &QT[(i-1)*n], 1, &A[j], n);
			double factor = 1.0 * nom / denom / denom;
			// u_j = u_j - proj_{u_i}a_j
			cblas_daxpy(n, -factor, &QT[(i-1)*n], 1, &QT[j*n], 1);
		}
	}
	// normalize Q^T
	for (int i = 0; i < n; i++){
		double oneOverNorm = 1.0 / cblas_dnrm2(n, &QT[i*n], 1);
		cblas_dscal(n, oneOverNorm, &QT[i*n], 1);
	}

	std::vector<double> b_new(n, 0);
	cblas_dgemv(CblasRowMajor, CblasNoTrans, n, n, 1.0, &QT[0], n, &b[0], 1, 0.0, &b_new[0], 1);

	// generating R 
	std::vector<double> R(n * n, 0);
	for (int i = 0; i < n; i++){
		for (int j = i; j < n; j++){
			if (i > j)
				R[i * n + j] = 0;
			else
				R[i * n + j] = cblas_ddot(n, &QT[i*n], 1, &A[j], n);
		}
	}
 
	// Back substitution
	for (int i = n - 1; i >= 0; i--){
		int idx = i * n + i + 1;
		double dot_product = cblas_ddot(n - i - 1, &R[idx], 1, &x[i+1], 1);
		//cout<<idx<<"   "<<A[idx-1]<<"   "<<dot_product<<endl;		
		x[i] = (b_new[i] - dot_product) / R[idx - 1];
	}
}



void CGSolver(std::vector<double> &A, int n, 
	std::vector<double> &b, std::vector<double> &x) {
	
	std::vector<double> r(n);
	std::vector<double> p(n);
	std::vector<double> Ap(n);
	cblas_dgemv(CblasRowMajor, CblasNoTrans, n, n, 1.0, &A[0],
			 n, &x[0], 1, 1.0, &r[0], 1);
	cblas_daxpy(n, -1.0, &b[0], 1, &r[0], 1);
	cblas_dcopy(n, &r[0], 1, &p[0], 1);
	cblas_dscal(n, -1.0, &p[0], 1);	

	double tol = 1e-8;	

	while (1){
		cblas_set_to_zero(Ap);
		cblas_dgemv(CblasRowMajor, CblasNoTrans, n, n, 1.0, &A[0],
			 n, &p[0], 1, 1.0, &Ap[0], 1);

		double nom = cblas_ddot(n, &r[0], 1, &r[0], 1);
		double denom = cblas_ddot(n, &p[0], 1, &Ap[0], 1);
		double alpha = nom / denom;
 
		cblas_daxpy(n, alpha, &p[0], 1, &x[0], 1);
		cblas_daxpy(n, alpha, &Ap[0], 1, &r[0], 1);

		double nomNew = cblas_ddot(n, &r[0], 1, &r[0], 1);
		double beta = nomNew / nom;

		cblas_dscal(n, beta, &p[0], 1);		
		cblas_daxpy(n, -1.0, &r[0], 1, &p[0], 1);

		//cout<<sqrt(nomNew)<<endl;
		if (sqrt(nomNew) <= tol)
			break;

	}

}
