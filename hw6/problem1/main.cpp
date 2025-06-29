// Bryan Ng, 2427348

#include "matrices.h"

// Function to return the minor of a matrix for which we want to calculate the determinant
// Omits the first (zeroth) row and the kth column
// Input: gsl_matrix * m = nxn matrix; int n = number of rows and columns for m; 
// int k: index of column to omit, ranges from 0 to n-1
void setMinor(gsl_matrix * m, int n, int k, gsl_matrix * minor) {

	int i,j;

	// printf("\nSetting minor...");

	for(i=0;i<(n-1);i++) {

		// printf("\ni = %d", i);


		for(j=0; j<(n-1);j++) {

			// printf("\nj = %d", j);

			if(j<k){

				gsl_matrix_set(minor, i, j, gsl_matrix_get(m, i+1, j));

			} else {

				gsl_matrix_set(minor, i, j, gsl_matrix_get(m, i+1, j+1));

			}
		}
	}

}

// Function that inputs an nxn gsl_matrix * and returns the determinant as a double
double getDeterminant(gsl_matrix * m, int n)
{

	double a;
	double det = 0;
	int k;

	// printf("\n\nGetting determinant...\n");
	// printf("n = %d \n", n);

	// Edge cases
	if(n == 1) {

		return(gsl_matrix_get(m, 0,0));

	}
	if(n == 2) {

		// printf("Calculated determinant!\n");

		return(gsl_matrix_get(m,0,0)*gsl_matrix_get(m,1,1) - gsl_matrix_get(m,0,1)*gsl_matrix_get(m,1,0));

	}

	gsl_matrix * minor = gsl_matrix_alloc(n-1, n-1);

	// Calculate determinant provided by formula
	for(k=0; k<n; k++) {

		// printf("\nk = %d", k);

		a = gsl_matrix_get(m, 0, k)*pow(-1, (double)k);

		setMinor(m, n, k, minor);

		det += a * getDeterminant(minor, n-1);

	}
	gsl_matrix_free(minor);

	return(det);

}


int main()
{
	
	// Number of rows of the symmetric matrix for which we want to calculate the determinant
	int n = 10;
	double det;

	// Read in banded matrix
	gsl_matrix * m = gsl_matrix_alloc(n, n);
	FILE * f = fopen("mybandedmatrix.txt", "r");
	gsl_matrix_fscanf(f, m);
	fclose(f);

	// Calculate determinant
	det = getDeterminant(m, n);

	// Free memory
	gsl_matrix_free(m);

	// Print determinant to console
	printf("The determinant of the matrix is: %.4lf\n", det);

	return(1);
}
