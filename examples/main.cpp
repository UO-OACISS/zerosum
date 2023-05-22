/*
 * MIT License
 *
 * Copyright (c) 2023 University of Oregon, Kevin Huck
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#ifdef USE_MPI
#include <mpi.h>
#define MPI_INIT  MPI_Init(&argc, &argv);
#define MPI_FINI  MPI_Finalize();
#define UNUSED(expr)
#else
#define MPI_INIT
#define MPI_FINI
#define UNUSED(expr) do { (void)(expr); } while (0)
#endif

void init_matrix(int n, double *a[]){
#pragma omp parallel for
    for(int k = 0; k < n - 1; ++k) {
        // for the vectoriser
        for(int i = k + 1; i < n; i++) {
            a[i][k] = i+k;
        }
    }
}

void lup_od_omp(int n, double *a[]){
    int i,k;

    for(k = 0; k < n - 1; ++k) {
        // for the vectoriser
        for(i = k + 1; i < n; i++) {
            a[i][k] /= a[k][k];
        }

        #pragma omp parallel for shared(a,n,k) private(i) schedule(runtime)
        for(i = k + 1; i < n; i++) {
            int j;
            const double aik = a[i][k]; // some compilers will do this automatically
            for(j = k + 1; j < n; j++) {
                a[i][j] -= aik * a[k][j];
            }
        }
    }
}

int main(int argc, char *argv[]){

    /* Set up MPI */
    MPI_INIT;
    UNUSED(argc);
    UNUSED(argv);

    /* do some work */
#ifdef NDEBUG
    constexpr int n = 1024*4;
#else
    constexpr int n = 1024*2;
#endif
    double * matrix[n];
    for (int i = 0 ; i < n ; i++) {
        matrix[i] = (double*)malloc(sizeof(double)*n);
    }
    init_matrix(n,matrix);
    lup_od_omp(n, matrix);

    /* Finalize MPI */
    MPI_FINI;

    return 0;
}
