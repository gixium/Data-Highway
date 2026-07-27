/* Programma per illustrare il funzionamento di callgrind / massif / perf *
 * contiene semplici operazioni di algebra lineare*/

#include <stdio.h>
#include <stdlib.h>
#define LEN 100 

int scalar_prod(int v1[], int v2[], unsigned int len){  
   int res = 0;
      for(int i=0; i< len; i++){
          res = res + v1[i] * v2[i];
      }
   return res;
}

void matmul(int m1[LEN][LEN], int m2[LEN][LEN],int m1m2[LEN][LEN]){
   for(int i=0; i < LEN; i++){
      for(int j=0; j < LEN; j++){
         m1m2[i][j] = 0;
	 for (int k = 0; k < LEN; k++){
	    m1m2[i][j] = m1[i][k] * m2 [k][j];
	 }
      }
   }
}

void slower_matmul(int m1[LEN][LEN], int m2[LEN][LEN],int m1m2[LEN][LEN]){
   int *v;
   v = (int*) malloc(LEN*sizeof(int));
   for(int i=0; i < LEN; i++){
      for(int j=0; j < LEN; j++){
	 for (int k=0; k < LEN ; k++){
	   v[k] = m2[k][j];
	 }
         m1m2[i][j] = scalar_prod(m1[i],v, LEN);
      }
   }
   free(v); 
}

void pretty_print_mat(int m[LEN][LEN]){
   for(int i=0; i < LEN; i++){
      printf("[");
      for(int j=0; j < LEN; j++){
        printf(" %3.d ",m[i][j]);
      }
      printf("]\n");
   }      
}

int main(int argc, char* argv[]){
   int m1[LEN][LEN] = {{0}}, 
       m2[LEN][LEN] = {{0}}, 
       m1m2[LEN][LEN] = {{0}};
   m1[LEN-1][LEN-1]=7;

   for(int i=0; i < LEN; i++){
      for(int j=0; j < LEN; j++){
         m1[i][j] = rand() % 128;
         m2[i][j] = rand() % 128;
      }
   }
   pretty_print_mat(m1);
   slower_matmul(m1,m2,m1m2);
   matmul(m1,m2,m1m2);
   return 0;
}
