/* splotch-relax lifts all inhibited activations in .Act-*.? files
   gradually up again 
   
   L. Schomaker
   */
   
#include <stdio.h>
#include <stdlib.h>

#define N         10000
#define ASYMPTOTE 1.0

int main(int argc, char *argv[]) 
{
	double xin[N], yout, alpha;
	int i, n;
	FILE *fp;
	
	if(argc != 3) {
		fprintf(stderr,"Usage: splotch-relax [alpha] [.Actfile]\n");
		exit(1);
	}
	
	fp = fopen(argv[2],"r");
	if(fp == NULL) {
		fprintf(stderr,"Error opening [.Actfile] %s\n", argv[2]);
		exit(1);
	}
	
	alpha = atof(argv[1]);
	
	fscanf(fp,"%d", &n);
	i = 0;
	while(fscanf(fp, "%lf", &xin[i]) != EOF) {
	    ++i;
	}
	fclose(fp);
	n = i;

	fp = fopen(argv[2],"w");
	if(fp != NULL) {
		fprintf(fp,"%d\n", n);
		for(i = 0; i < n; ++i) {
			yout = alpha * ASYMPTOTE + (1. - alpha) * xin[i];
			fprintf(fp,"%5.2f\n", yout);
		}
		fclose(fp);
	}
}
