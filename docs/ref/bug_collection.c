/* Programma per illustrare l'efficacia di ASAN/valgrind 
 * contiene una raccolta di bug subdoli */

#include <stdio.h>
#include <stdlib.h>   

int scalar_prod(int v1[], int v2[], unsigned int len){  
   int res = 0;
      for(int i=0; i < len; i++){
          res = res + v1[i] * v2[i];
      }
   return res;
}


int scalar_prod_2(int v1[], int v2[], unsigned int len, int * res){  
      for(int i=0; i < len; i++){
          *res = *res + v1[i] * v2[i];
      }
   return *res;
}

void rand_fill(int *v, unsigned int len){
    for(int i=0; i <= len; i++){
        v[i] = rand() % 128;
    }
}

void one_fill(int *v, unsigned int len){
    for(int i=0; i < len; i++){
        v[i] = 1;
    }
}


#define LEN 1000
int main (int argc,char* argv[]){
   int v1[LEN],v2[LEN];
   int v3[LEN]={0}, ps=0;
   one_fill(v1,LEN);
   one_fill(v2,LEN);
   // ps = scalar_prod(v1,v2,LEN);
   // printf("il prodotto scalare è: %d\n",ps);
   scalar_prod_2(v1,v2,LEN,&ps);
   printf("il prodotto scalare è: %d\n",ps);
   one_fill(v3,LEN);
   return 0;
}
