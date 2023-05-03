/**********************************************************
"Hello World"-type program to test different srun layouts.

Written by Tom Papatheodore
**********************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

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

        #pragma omp parallel for shared(a,n,k) private(i) schedule(static, 64)
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
    MPI_Init(&argc, &argv);

    /* do some work */
    constexpr int n = 3000;
    double * matrix[n];
    for (int i = 0 ; i < n ; i++) {
        matrix[i] = (double*)malloc(sizeof(double)*n);
    }
    init_matrix(n,matrix);
    lup_od_omp(n, matrix);

    /* Finalize MPI */
    MPI_Finalize();

    return 0;
}
