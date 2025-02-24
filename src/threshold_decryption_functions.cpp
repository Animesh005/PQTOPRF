#include "threshold_decryption_functions.hpp"
#include "threshold_decryption_vars.hpp"

int ncrT(int n, int r){
    if (ncr_cacheT.find({n, r}) == ncr_cacheT.end()){
        if (r > n || n < 0 || r < 0)
            return 0;
        else{
            if (r == 0 || r == n){
                ncr_cacheT[{n, r}] = 1;
            }else if (r == 1 || r == n - 1){
                ncr_cacheT[{n, r}] = n;
            }else{
                ncr_cacheT[{n, r}] = ncrT(n - 1, r) + ncrT(n - 1, r - 1);
            }
        }
    }
    return ncr_cacheT[{n, r}];
}

/* B is a distribution matrix for any single variable x1. So B is a identity matrix of dimension k. A is a distribution matrix of form x1x2...xi. This method returns distribution matrix for x1x2...x(i+1)*/
ublas::matrix<int> andCombineT(ublas::matrix<int>& A, ublas::matrix<int>& B, int k){
	int rA = A.size1(); int cA = A.size2();
	int rB = B.size1(); int cB = B.size2();
	int r = rA + rB; int c = cA + cB;
	ublas::matrix<int> C;
	C = ublas::zero_matrix<int>(r, c);

	#pragma omp parallel for collapse(2)
	for(int col = 0; col < k; col++){
		for(int row = 0; row < rA; row++){
			C(row, col) = A(row, col);
			C(row, col + k) = A(row, col);
		}
	}
	#pragma omp barrier

	#pragma omp parallel for collapse(2)
	for(int col = k; col < 2*k; col++){
		for(int row = rA; row < r; row++){
			C(row, col) = B(row - rA, col - k);
		}
	}
	#pragma omp barrier

	#pragma omp parallel for collapse(2)
	for(int col = 2*k; col < c; col++){
		for(int row = 0; row < rA; row++){
			C(row, col) = A(row, col - k);
		}
	}
	#pragma omp barrier

	A.resize(r, c);
	A = C;
	return A;
}

/* B is distribution matrix of t-sized AND form x1x2...xt. A is distribution matrix of OR-ing i number of such x1x2...xt terms. This method returns distribution matrix of OR-ing (i+1) terms of the form x1x2...xt. */
ublas::matrix<int> orCombineT(ublas::matrix<int>& A, ublas::matrix<int>& B, int k){
	int rA = A.size1(); int cA = A.size2();
	int rB = B.size1(); int cB = B.size2();
	int r = rA + rB; int c = cA + cB - k;
	ublas::matrix<int> C;
	C = ublas::zero_matrix<int>(r, c);

	#pragma omp parallel for collapse(2)
	for(int col = 0; col < k; col++){
		for(int row = 0; row < r; row++){
			if (row < rA){
				C(row, col) = A(row, col);
			}else{
				C(row, col) = B(row - rA, col);
			}
		}
	}
	#pragma omp barrier

	#pragma omp parallel for collapse(2)
	for(int col = k; col < cA; col++){
		for(int row = 0; row < rA; row++){
			C(row, col) = A(row, col);
		}
	}
	#pragma omp barrier

	#pragma omp parallel for collapse(2)
	for(int col = cA; col < c; col++){
		for(int row = rA; row < r; row++){
			C(row, col) = B(row - rA, col - cA + k);
		}
	}
	#pragma omp barrier


	A.resize(r, c);
	A = C;
	return A;
}

// Copies the entirety of src matrix inside dst
// starting from the index (dstR, dstC) in dst
void matrixCopy(ublas::matrix<int> &dst, ublas::matrix<int> &src, int dstR, int dstC){
	// for (int r = 0; r < src.size1(); r++){
	// 	for (int c = 0; c < src.size2(); c++){
	// 		dst(dstR + r, dstC + c) = src(r, c);
	// 	}
	// }
	ublas::matrix_range<ublas::matrix<int>> dstSlice (dst, ublas::range(dstR, dstR + src.size1()), ublas::range(dstC, dstC + src.size2()));
	dstSlice = src;
}

ublas::matrix<int> optAndCombineT(int t, int k){
	ublas::matrix<int> I;
	I = ublas::identity_matrix<int>(k);

	ublas::matrix<int> Mf;
	int kt = k*t;
	Mf = ublas::zero_matrix<int>(kt, kt);

	// Divide the matrix into t*t chunks of k*k sub-matrices
	for (int r = 0; r < t; r++){
		for (int c = 0; c < t; c++){
			if (r == 0 || c == t - r){
				matrixCopy(Mf, I, r*k, c*k);
			}
		}
	}

	return Mf;
}

ublas::matrix<int> optOrCombineT(int k, int t, int l, ublas::matrix<int> &A){
	ublas::matrix<int> F, R;
	F = ublas::zero_matrix<int>(A.size1(), k);
	R = ublas::zero_matrix<int>(A.size1(), A.size2() - k);

	for (int r = 0; r < A.size1(); r++){
		for (int c = 0; c < A.size2(); c++){
			if (c < k){
				F(r, c) = A(r, c);
			}else{
				R(r, c - k) = A(r, c);
			}
		}
	}

	ublas::matrix<int> M;
	M = ublas::zero_matrix<int>(l*k*t, k*(t-1) * l + k);
	for (int i = 0; i < l; i++){
		matrixCopy(M, F, i*k*t, 0);
		matrixCopy(M, R, i*k*t, k + i*k*(t-1));
	}

	return M;
}

/* Build Distribution matrix for OR-ing C(p,t) number of x1x2...xt like terms. */
void buildDistributionMatrix(int t, int k, int p, ublas::matrix<int>& M){
	// ublas::matrix<int> M2;
	// M2 = ublas::identity_matrix<int>(k);
	ublas::matrix<int> M1;
	// M1 = M2;
	// for(int i = 2; i <= t; i++){
	// 	M1 = andCombineT(M1, M2, k);
	// }
	M1 = optAndCombineT(t, k);
	M = optOrCombineT(k, t, ncrT(p, t), M1);
	// for(int i = 2; i <= ncrT(p, t); i++){
	// 	M = orCombineT(M, M1, k);
	// }
}

/* rho is a random binary matrix with first k rows coming from the k rows of the secret key */
ublas::matrix<int> buildRho(int k, int p, int e, TLweKey *key, ublas::matrix<int>& rho){
	int N = key->params->N;
	rho = ublas::matrix<int>(e, N);
	for(int row = 0; row < k; row++){
		for(int col = 0; col < N; col++){
			rho(row,col) = key->key[row].coefs[col];
		}
	}
	std::default_random_engine gen;
    std::uniform_int_distribution<int> dist(0, 1);
	for(int row = k; row < e; row++){
		for(int col = 0; col < N; col++){
			rho(row,col) = dist(gen);
		}
	} 
	return rho;
}

/* Naive Matrix Multiplication */
void multiply(ublas::matrix<int>& C, ublas::matrix<int>& A, ublas::matrix<int>& B){
	int m = A.size1();
	int k = A.size2();
	int n = B.size2();

	double *_A = (double *)malloc(m * k * sizeof(double));
	double *_B = (double *)malloc(k * n * sizeof(double));
	double *_C = (double *)malloc(m * n * sizeof(double));

	for (int r = 0; r < m; r++)
		for (int c = 0; c < k; c++)
			_A[r * k + c] = A(r, c);

	for (int r = 0; r < k; r++)
		for (int c = 0; c < n; c++)
			_B[r * n + c] = B(r, c);

	memset(_C, 0, m * n * sizeof(double));

	cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, 1.0, _A, k, _B, n, 0.0, _C, n);

	for (int r = 0; r < m; r++)
		for (int c = 0; c < n; c++)
			C(r, c) = _C[r * n + c];

	free(_A);
	free(_B);
	free(_C);
}

/* Given a group_id, find the party_ids present in (group_id)^th combination out of C(p,t) combinations */
void findParties(std::vector<int>& pt, int gid, int t, int p){
	int mem = 0, tmp;
	pt.clear();
	for(int i = 1; i < p; i++){
		tmp = ncrT(p - i, t - mem -1);
		if(gid > tmp){
			gid -= tmp;
		}
		else{
			pt.push_back(i);
			mem += 1;
		}
		if(mem + (p-i) == t){
			for(int j = i + 1; j <= p; j++){
				pt.push_back(j);
			}
			break;
		}
	}
}


void shareSecret(int t, int p, const LweKey *key, const LweParams *params){
	
	int k = 1;
	int N = params->n;
	std::default_random_engine gen;
    std::uniform_int_distribution<int> dist(0, 1);
	long long int group_count = ncrT(p,t);
	long long int d = group_count*t*k;

	/* We store only k(t-1) rows of rho matrix at a time during on-the-fly computation of key shares corresponding to one t-sized group*/
	int **partial_rho = (int**)malloc(k * (t - 1) * sizeof(int*));
	for(int i = 0; i < k * ( t- 1); i++){
		partial_rho[i] = (int*)malloc(N * sizeof(int));
	}

	for(int group_id = 1; group_id <= group_count; group_id++){
		std::vector<int> parties;
		findParties(parties, group_id, t, p);
		for(int i = 0; i < k*(t-1); i++){
			for(int j = 0; j < N; j++){
				partial_rho[i][j] = dist(gen);
			}
		}
		for(int i = 0; i < parties.size(); i++){
			LweKey *key_share = new_LweKey(params);
			if(i == 0){
				for(int keyi = 0; keyi < k; keyi++){
					for(int keyj = 0; keyj < N; keyj++){
						key_share->key[keyj] = key->key[keyj];
					}
				}
				for(int keyi = 0; keyi < k; keyi++){
					for(int row = keyi; row < k*(t-1); row+=k){
						for(int j = 0; j < N; j++){
							key_share->key[j] += partial_rho[row][j];
						}
					}
				}
			}
			else{
				for(int keyi = 0, row = k*(t-1-i); keyi < k; keyi++, row++){
					for(int keyj = 0; keyj < N; keyj++){
						key_share->key[keyj] = partial_rho[row][keyj];
					}
				}
			}
			shared_key_repo[{parties[i], group_id}] = key_share;
		}
	}
}

/* Given a t-sized list of party-ids compute its rank among total C(p,t) combinations */
int findGroupId(std::vector<int> parties, int t, int p){
	int mem = 0;
	int group_count = 1;
	for(int i = 1; i <= p; i++){
		if(std::find(parties.begin(), parties.end(), i) != parties.end()){
			mem += 1;
		}
		else{
			group_count += ncrT(p - i, t - mem - 1);
		}
		if(mem == t){
			break;
		}
	}
	return group_count;
}

Torus32 thresholdDecrypt(LweSample *ciphertext, const LweParams* params, std::vector<int> parties, int t, int p, double sd){
	int k = 1;
	int N = params->n;
	
	int group_id = findGroupId(parties, t, p);
	Torus32 acc = 0;
	
	for(int i = 0; i < t; i++){
		Torus32 tmp = 0;
		
		auto part_key = shared_key_repo[{parties[i], group_id}];
		
		for(int32_t j = 0; j < N; ++j){
	        tmp += ciphertext->a[j]*part_key->key[j]; 
		}

		tmp += gaussian32(0, sd);
		if(i == 0){
			acc += tmp;
		}
		else{
			acc -= tmp;
		}
	}

	acc -= ciphertext->b;
	acc = -acc;
	return acc;
}