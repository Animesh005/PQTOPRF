#pragma once

#include <iostream>
#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include <tfhe/lwe-functions.h>
#include <tfhe/numeric_functions.h>
#include <tfhe/tlwe_functions.h>
#include <random>
#include <bits/stdc++.h>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/blas.hpp>
#include <omp.h>
#include <cblas.h>

#define MSIZE 2
namespace ublas = boost::numeric::ublas;

void shareSecret(int t, int p, const LweKey *key, const LweParams *params);
Torus32 thresholdDecrypt(LweSample *ciphertext, const LweParams* params, std::vector<int> parties, int t, int p, double sd);
int ncrT(int n, int r);