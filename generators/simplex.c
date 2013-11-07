#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define F2 0.5*(sqrt(3.0)-1.0)
#define G2 (3.0-sqrt(3.0))/6.0
#define F3 1.0/3.0
#define G3 1.0/6.0
#define F4 (sqrt(5.0)-1.0)/4.0
#define G4 (5.0-sqrt(5.0))/20.0

typedef struct {
   double x, y, z;
} Grad3;

typedef struct {
   double x, y, z, w;
} Grad4;

double noise2D(double, double, short*, short*, Grad3*);

void _write(char* fname, double *data, short MESH_SIZE) {
   FILE* fp_img = NULL;
   if (!(fp_img = fopen(fname, "wb"))) {
      printf("Error with mesh file.\n");
      return;
   }
   long long int img_written = fprintf(fp_img, "P2 %d %d 255", MESH_SIZE, MESH_SIZE);
   for (int i = 0; i < MESH_SIZE; i++) {
      img_written += fprintf(fp_img, "\n");
      for (int j = 0; j < MESH_SIZE; j++) {
         double t = data[i + (j*MESH_SIZE)];
         img_written += fprintf(fp_img, "%3d ", 255-(int)(t * 255));
      }
   }
   fclose(fp_img);
}

bool in(short* arr, short val, int size) {
   int i=0;
   while(i<size) {
      if (arr[i] == val) {
         return 1;
      }
      ++i;
   }
   return 0;
}

short* genPermutations(int low, int high) {
   int range = high - low + 1;
   short* p = (short*) calloc(sizeof(short),range);
   int i = 1;
   srand(time(NULL));
   short v = rand() % range;
   p[0] = v;
   while (i<range) {
      v = rand() % range;
      while (in(p,v,i) == 1) {
         v = rand() % range;
      }
      p[i] = v;
      ++i;
   }
   return p;
}

int fastFloor(double x) {
   int xx = (int)x;
   return x<xx ? xx-1 : xx;
}

double dot2(Grad3 g, double x, double y) { return g.x*x + g.y*y; }
double dot3(Grad3 g, double x, double y, double z) { return g.x*x + g.y*y + g.z*z; }
double dot4(Grad4 g, double x, double y, double z, double w) { return g.x*x + g.y*y + g.z*z + g.w*w; }

double noise(double x, double y, short* perm, short* permMod12, Grad3* gradients, int octaves, int persistence) {
   double total = 0;
   for (int i=0;i<octaves-1;++i) {
      double freq = pow(2.0,i);
      double amp = pow(persistence,i);
      total += noise2D(x*freq,y*freq,perm,permMod12,gradients) * amp;
   }
   return total;
}

double noise2D(double x, double y, short* perm, short* permMod12, Grad3* gradients) {
   double n0,n1,n2;
   double s = (x+y)*F2;
   int i = fastFloor(x+s);
   int j = fastFloor(y+s);
   double t = (i+j)*G2;
   double X0 = i-t;
   double Y0 = j-t;
   double x0 = x-X0;
   double y0 = y-Y0;
   int i1, j1;
   if (x0>y0) {i1=1;j1=0;}
   else {i1=0;j1=1;}
   double x1 = x0 - i1 + G2;
   double y1 = y0 - j1 + G2;
   double x2 = x0 - 1.0 + 2.0 * G2;
   double y2 = y0 - 1.0 + 2.0 * G2;
   int ii = i & 255;
   int jj = j & 255;
   int gi0 = permMod12[ii+perm[jj]];
   int gi1 = permMod12[ii+i1+perm[jj+j1]];
   int gi2 = permMod12[ii+1+perm[jj+1]];
   double t0 = 0.5 - x0*x0-y0*y0;
   if(t0<0) n0 = 0.0;
   else {
      t0 *= t0;
      n0 = t0 * t0 * dot2(gradients[gi0], x0, y0);
   }
   double t1 = 0.5 - x1*x1-y1*y1;
   if(t1<0) n1 = 0.0;
   else {
      t1 *= t1;
      n1 = t1 * t1 * dot2(gradients[gi1], x1, y1);
   }
   double t2 = 0.5 - x2*x2-y2*y2;
   if(t2<0) n2 = 0.0;
   else {
      t2 *= t2;
      n2 = t2 * t2 * dot2(gradients[gi2], x2, y2);
   }
   return 70.0 * (n0 + n1 + n2);
}


int main(int argc, char** argv) {
   int MESH_SIZE = 64;
   int FREQ = 5;
   float PERS = 0.5;
   float POINT_DIST = 0.005;
   if( argc != 5 ) {
      printf( "%s MESH_SIZE FREQ PERS PDIST\n", argv[0]);
      return 0;
   }
   sscanf(argv[1],"%d",&MESH_SIZE );
   sscanf(argv[2],"%d",&FREQ );
   sscanf(argv[3],"%f",&PERS );
   sscanf(argv[4],"%f",&POINT_DIST );

   short* p = genPermutations(0,255);
   short* perm = (short*)calloc(sizeof(short),512);
   short* permMod12 = (short*)calloc(sizeof(short),512);

   // Generate permutations from 0 to 255 inclusive.  Do this twice for wrapping.
   for (int i=0;i<512;++i) {
      perm[i]=p[i & 255];
      permMod12[i] = (short)(perm[i] % 12);
   }

   Grad3* gradients3D = (Grad3*)malloc(12 * sizeof(Grad3));
   // Generate gradients
   int cnt = 0;
   for (int i=-1;i<2;i+=2) {
      for (int j=-1;j<2;j+=2) {
         gradients3D[cnt].x=i;
         gradients3D[cnt].y=j;
         gradients3D[cnt++].z=0;
      }
   }
   for (int i=-1;i<2;i+=2) {
      for (int j=-1;j<2;j+=2) {
         gradients3D[cnt].x=i;
         gradients3D[cnt].z=j;
         gradients3D[cnt++].y=0;
      }
   }
   for (int i=-1;i<2;i+=2) {
      for (int j=-1;j<2;j+=2) {
         gradients3D[cnt].y=i;
         gradients3D[cnt].z=j;
         gradients3D[cnt++].x=0;
      }
   }

   double* narr = (double*)malloc(MESH_SIZE*MESH_SIZE*sizeof(double));
   for (int i=0;i<MESH_SIZE;++i) {
      for (int j=0;j<MESH_SIZE;++j) {
         narr[i+j*MESH_SIZE] = noise(i*POINT_DIST,j*POINT_DIST,perm,permMod12,gradients3D,FREQ,PERS);
      }
   }

   _write("out.pgm",narr,MESH_SIZE);

   free(narr);
   free(p);
   free(gradients3D);
   free(perm);
   free(permMod12);
   return 0;
}
