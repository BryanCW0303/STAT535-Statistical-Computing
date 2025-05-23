// Bryan Ng, 2427348

#include "matrices.h"

//prints the elements of a matrix in a file
void printmatrix(char* filename,gsl_matrix* m)
{
	int i,j;
	double s;
	FILE* out = fopen(filename,"w");
	
	if(NULL==out)
	{
		printf("Cannot open output file [%s]\n",filename);
		exit(1);
	}
	for(i=0;i<m->size1;i++)
	{
	        fprintf(out,"%.3lf",gsl_matrix_get(m,i,0));
		for(j=1;j<m->size2;j++)
		{
			fprintf(out,"\t%.3lf",
				gsl_matrix_get(m,i,j));
		}
		fprintf(out,"\n");
	}
	fclose(out);
	return;
}

//creates the transpose of the matrix m
gsl_matrix* transposematrix(gsl_matrix* m)
{
	int i,j;
	
	gsl_matrix* tm = gsl_matrix_alloc(m->size2,m->size1);
	
	for(i=0;i<tm->size1;i++)
	{
		for(j=0;j<tm->size2;j++)
		{
		  gsl_matrix_set(tm,i,j,gsl_matrix_get(m,j,i));
		}
	}	
	
	return(tm);
}

//calculates the product of a nxp matrix m1 with a pxl matrix m2
//returns a nxl matrix m
void matrixproduct(gsl_matrix* m1,gsl_matrix* m2,gsl_matrix* m)
{
	int i,j,k;
	double s;
	
	for(i=0;i<m->size1;i++)
	{
	  for(k=0;k<m->size2;k++)
	  {
	    s = 0;
	    for(j=0;j<m1->size2;j++)
	    {
	      s += gsl_matrix_get(m1,i,j)*gsl_matrix_get(m2,j,k);
	    }
	    gsl_matrix_set(m,i,k,s);
	  }
	}
	return;
}


//computes the inverse of a positive definite matrix
//the function returns a new matrix which contains the inverse
//the matrix that gets inverted is not modified
gsl_matrix* inverse(gsl_matrix* K)
{
	int j;
	
	gsl_matrix* copyK = gsl_matrix_alloc(K->size1,K->size1);
	if(GSL_SUCCESS!=gsl_matrix_memcpy(copyK,K))
	{
		printf("GSL failed to copy a matrix.\n");
		exit(1);
	}
	
	gsl_matrix* inverse = gsl_matrix_alloc(K->size1,K->size1);
	gsl_permutation *myperm = gsl_permutation_alloc(K->size1);
	
	if(GSL_SUCCESS!=gsl_linalg_LU_decomp(copyK,myperm,&j))
	{
		printf("GSL failed LU decomposition.\n");
		exit(1);
	}
	if(GSL_SUCCESS!=gsl_linalg_LU_invert(copyK,myperm,inverse))
	{
		printf("GSL failed matrix inversion.\n");
		exit(1);
	}
	gsl_permutation_free(myperm);
	gsl_matrix_free(copyK);
	
	return(inverse);
}

//creates a submatrix of matrix M
//the indices of the rows and columns to be selected are
//specified in the last four arguments of this function
gsl_matrix* MakeSubmatrix(gsl_matrix* M,
			  int* IndRow,int lenIndRow,
			  int* IndColumn,int lenIndColumn)
{
	int i,j;
	gsl_matrix* subM = gsl_matrix_alloc(lenIndRow,lenIndColumn);
	
	for(i=0;i<lenIndRow;i++)
	{
		for(j=0;j<lenIndColumn;j++)
		{
			gsl_matrix_set(subM,i,j,
                                       gsl_matrix_get(M,IndRow[i],IndColumn[j]));
		}
	}
	
	return(subM);
}


//computes the log of the determinant of a symmetric positive definite matrix
double logdet(gsl_matrix* K)
{
        int i;

	gsl_matrix* CopyOfK = gsl_matrix_alloc(K->size1,K->size2);
	gsl_matrix_memcpy(CopyOfK,K);
	gsl_permutation *myperm = gsl_permutation_alloc(K->size1);
	if(GSL_SUCCESS!=gsl_linalg_LU_decomp(CopyOfK,myperm,&i))
	{
		printf("GSL failed LU decomposition.\n");
		exit(1);
	}
	double logdet = gsl_linalg_LU_lndet(CopyOfK);
	gsl_permutation_free(myperm);
	gsl_matrix_free(CopyOfK);
	return(logdet);
}

// Computes the log marginal likelihood of the normal linear regression with inverse gamma prior
// Inputs: gsl_matrix* data = data matrix; int lenA = number of covariates; int* A = indices of covariates
// Outputs: double lml = log marginal likelihood

double marglik(gsl_matrix* data,int lenA,int* A)
{
	double lml;

	int i,j;

	int n = data -> size1;
	int p = data -> size2;

	// Initialize vector corresponding to rows needed for MakeSubmatrix
	int * rows = new int[n];
	for(i=0;i<n;i++) {

		rows[i]=i;
	}

	// Initialize vector corresponding to columns needed for MakeSubmatrix for d1
	int * cols1 = new int[1];
	cols1[0] = 0;

	// Initialize vector corresponding to columns needed for MakeSubmatrix for dA
	int * colsA = new int[lenA];
	for(j=0;j<lenA;j++) {

		colsA[j] = A[j]-1; // C indexes at 0

	}

	// Make d1
	gsl_matrix* d1 = gsl_matrix_alloc(1,n);
	d1 = MakeSubmatrix(data, rows, n, cols1, 1); 
	// printmatrix("d1.mat",d1);

	// Make dA
	gsl_matrix* dA = gsl_matrix_alloc(lenA, n);
	dA = MakeSubmatrix(data, rows, n, colsA, lenA);
	// printmatrix("dA.mat",dA);

	// Make tdA
	gsl_matrix* tdA = transposematrix(dA);

	// Make identity matrix with dim lenA
	gsl_matrix* AId = gsl_matrix_alloc(lenA,lenA);
	gsl_matrix_set_identity(AId);

	// Make mA
	gsl_matrix* mA = gsl_matrix_alloc(lenA,lenA);
	matrixproduct(tdA, dA, mA);
	gsl_matrix_add(mA, AId);
	// printmatrix("mA.mat",mA);

	// Make td1
	gsl_matrix* td1 = transposematrix(d1);

	// Make td1 * d1
	gsl_matrix* td1_d1 = gsl_matrix_alloc(1,1);
	matrixproduct(td1, d1, td1_d1);

	// Make m inverse
	gsl_matrix* mAInverse = inverse(mA);

	// Create matrix to hold td1*dA
	gsl_matrix* td1_dA = gsl_matrix_alloc(1,3);
	matrixproduct(td1, dA, td1_dA);

	// Create matrix to hold td1*dA*mAInverse
	gsl_matrix* td1_dA_mAInverse = gsl_matrix_alloc(1,3);
	matrixproduct(td1_dA, mAInverse, td1_dA_mAInverse);

	// Create matrix to hold tdA*d1
	gsl_matrix* tdA_d1 = gsl_matrix_alloc(3,1);
	matrixproduct(tdA, d1, tdA_d1);

	// Create matrix to hold td1*dA*mAInverse*tdA*d1)
	gsl_matrix* td1_dA_mAInverse_tdA_d1 = gsl_matrix_alloc(1,1);
	matrixproduct(td1_dA_mAInverse, tdA_d1, td1_dA_mAInverse_tdA_d1);

	// Calculate log marginal likelihood
	lml = (lgamma((n + lenA + 2.0)/2.0) - lgamma((lenA + 2.0)/2.0) - (1.0/2.0)*logdet(mA) - 
		((n + lenA + 2.0)/2.0)*log(1.0 + gsl_matrix_get(td1_d1, 0, 0) - 
		gsl_matrix_get(td1_dA_mAInverse_tdA_d1, 0, 0)));


	// Deallocate memory
	delete[] rows; rows = NULL;
	delete[] cols1; cols1 = NULL;
	delete[] colsA; colsA = NULL;
	gsl_matrix_free(d1);
	gsl_matrix_free(dA);
	gsl_matrix_free(tdA);
	gsl_matrix_free(AId);
	gsl_matrix_free(mA);
	gsl_matrix_free(td1);
	gsl_matrix_free(td1_d1);
	gsl_matrix_free(mAInverse);
	gsl_matrix_free(td1_dA);
	gsl_matrix_free(td1_dA_mAInverse);
	gsl_matrix_free(tdA_d1);
	gsl_matrix_free(td1_dA_mAInverse_tdA_d1);

	return(lml);


}



