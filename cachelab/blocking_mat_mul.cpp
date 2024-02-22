void bijk(array A, array B, array C, int n, int bsize)
{
    int i, j, k, kk, jj;
    double sum;
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < n; j++)
        {
            C[i][j] = 0;
        }
    }

    int en = n / bsize * bsize;

    for(kk = 0; kk < en; kk += bsize)
    {
        for(jj = 0; jj < en; jj += bsize)
        {
            for(i = 0; i < n; i++)
            {
                for(j = jj; j < jj + bsize; j++)
                {
                    sum = C[i][j];
                    for(k = kk; k < kk + bsize; k++)
                    {
                        sum += A[i][k] * B[k][j];
                    }
                    C[i][j] = sum;
                }
            }
        }
    }
}