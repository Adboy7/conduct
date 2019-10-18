#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
//NB: For me the top left cell is the {0,0} cell and the down right is the {N-1,N-1} cell.
#include "omp.h"

static int32_t s_randtbl[32] =
    {
        3,

        -1726662223,
        379960547,
        1735697613,
        1040273694,
        1313901226,
        1627687941,
        -179304937,
        -2073333483,
        1780058412,
        -1989503057,
        -615974602,
        344556628,
        939512070,
        -1249116260,
        1507946756,
        -812545463,
        154635395,
        1388815473,
        -1926676823,
        525320961,
        -1009028674,
        968117788,
        -123449607,
        1284210865,
        435012392,
        -2017506339,
        -911064859,
        -370259173,
        1132637927,
        1398500161,
        -205601318,
};

typedef struct Seed
{
    int32_t randtbl[32];
    int32_t *fptr;
    int32_t *rptr;
    int32_t *end_ptr;
    int32_t *state;
} Seed;

// The random function you could used in your project instead of the C 'rand' function.
unsigned int my_rand(Seed *seed)
{
    int32_t *fptr = seed->fptr;
    int32_t *rptr = seed->rptr;
    int32_t *end_ptr = seed->end_ptr;
    unsigned int val;

    val = *fptr += (unsigned int)*rptr;
    ++fptr;
    if (fptr >= end_ptr)
    {
        fptr = seed->state;
        ++rptr;
    }
    else
    {
        ++rptr;
        if (rptr >= end_ptr)
            rptr = seed->state;
    }
    seed->fptr = fptr;
    seed->rptr = rptr;

    return val;
}

void init_seed(Seed *seed)
{
    for (int i = 0; i < 32; ++i)
    {
        seed->randtbl[i] = s_randtbl[i];
    }
    seed->fptr = &(seed->randtbl[2]);
    seed->rptr = &(seed->randtbl[1]);
    seed->end_ptr = &(seed->randtbl[sizeof(seed->randtbl) / sizeof(seed->randtbl[0])]);
    seed->state = &(seed->randtbl[1]);

    unsigned int init = (time(NULL) << omp_get_thread_num());
    seed->state[0] = init;
    int32_t *dst = seed->state;
    int32_t word = init;
    int kc = 32;
    for (int i = 1; i < kc; ++i)
    {
        long int hi = word / 127773;
        long int lo = word % 127773;
        word = 16807 * lo - 2836 * hi;
        if (word < 0)
            word += 2147483647;
        *++dst = word;
    }
    seed->fptr = &(seed->state[3]);
    seed->rptr = &(seed->state[0]);
    kc *= 10;
    while (--kc >= 0)
    {
        my_rand(seed);
    }
}

void fiberfill(int *tab, int N, float d)
{
    int x, y, dir;
    int nbrFiber = d * N * N;

    Seed seed;
    init_seed(&seed);

    for (int i = 0; i < nbrFiber; i++)
    {
        //random number for x-y between 0 and N-1
        x = my_rand(&seed) % N;
        y = my_rand(&seed) % N;
        //random number for dir between 0 and 1
        dir = my_rand(&seed) % 2;

        //set the tab(x,y) cell to 1
        tab[y * N + x] = 1;

        //dir
        if (dir == 0)
        {

            if ((y * N + x) % N != 0)
            {
                tab[y * N + x - 1] = 1;
            }

            if ((y * N + x) % N != N - 1)
            {
                tab[y * N + x + 1] = 1;
            }
        }

        else
        {

            if (y * N + x >= N)
            {
                tab[(y - 1) * N + x] = 1;
            }

            if (y * N + x < N * (N - 1))
            {
                tab[(y + 1) * N + x] = 1;
            }
        }
    }
}

void connectedConducting(int *tab, int x, int y, int N)
{
    //if tab[y*N+x]==0 not conducting so we stop and if tab[y*N+x]==2
    //the cell is already connected so we also stop
    if (tab[y * N + x] == 1)
    {

        tab[y * N + x] = 2;

        //left cell
        if (x > 0)
        {
            connectedConducting(tab, x - 1, y, N);
        }

        //top cell
        if (y > 0)
        {
            connectedConducting(tab, x, y - 1, N);
        }

        //right cell
        if (x < N - 1)
        {
            connectedConducting(tab, x + 1, y, N);
        }

        //down cell
        if (y < N - 1)
        {
            connectedConducting(tab, x, y + 1, N);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc > 5 || argc<4)
    {
        fprintf(stderr, "Not enough or too much argument(s).\n");
        return 1;
    }
    else
    {
        int N, flag, M, nbrOfConducting;
        float d;
        nbrOfConducting = 0;
        flag = atoi(argv[1]);
        N = atoi(argv[2]);
        d = atof(argv[3]);

        if (flag == 1)
        {
            M = atof(argv[4]);
        }
        else
        {
            M = 1;
        }
#pragma omp parallel
{
#pragma omp for schedule(dynamic) reduction(+:nbrOfConducting)
        for (int i = 0; i < M; i++)
        {
            int *tab;
            //Initialisation of the N*N Tab.
            tab = malloc(N * N * sizeof(int));
            if (tab == NULL)
            {
                fprintf(stderr, "The allocation of memory failed.\n");
                exit(EXIT_FAILURE);
            }
            for (int i = 0; i < N * N; i++)
            {
                tab[i] = 0;
            }

            fiberfill(tab, N, d);

            for (int i = 0; i < N; i++)
            {
                connectedConducting(tab, 0, i, N);
            }

            for (int i = 0; i < N; i++)
            {
                if (tab[N * i + N - 1] == 2)
                {
                    if (flag == 0)
                    {
                        printf("The grid is conducting \n");
                    }
                    else
                    {
                        nbrOfConducting++;
                    }

                    break;
                }
                if (i == N - 1 && flag == 0)
                {

                    printf("The grid is not conducting \n");
                }
            }

            if (flag == 0)
            {
                //Writing the PPM file
                FILE *fp;
                fp = fopen("grid.ppm", "wb");
                if (fp == NULL)
                {
                    fprintf(stderr, "The openning/creation of grid.ppm failed\n");
                    exit(EXIT_FAILURE);
                }

                // Writing Magic Number to the File
                fprintf(fp, "P6\n");
                // Writing Width and Height
                fprintf(fp, "%d %d\n", N, N);
                // Writing the maximum gray value
                fprintf(fp, "255\n");
                // Writing the tab
                static unsigned char color[3];
                for (int i = 0; i < N; i++)
                {
                    for (int j = 0; j < N; j++)
                    {
                        if (tab[i * N + j] == 2)
                        {

                            color[0] = 200;
                            color[1] = 150;
                            color[2] = 0;
                            fwrite(color, 1, 3, fp);
                        }
                        if (tab[i * N + j] == 1)
                        {
                            color[0] = 0;
                            color[1] = 0;
                            color[2] = 0;
                            fwrite(color, 1, 3, fp);
                        }
                        if (tab[i * N + j] == 0)
                        {
                            color[0] = 50;
                            color[1] = 50;
                            color[2] = 50;
                            fwrite(color, 1, 3, fp);
                        }
                    }
                }
                fclose(fp);
            }

            free(tab);
        }
}

        if (flag == 1)
        {
            printf("The probability ofconduction is : %f\n", (float)nbrOfConducting / M);
        }
    }

exit(EXIT_SUCCESS);
}