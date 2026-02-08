	
#include <stdio.h>
#include <stdlib.h>
	
	
int main(){
	
	int nnodes = 14;
	int i;
	
	//Reservem memoria de la matriu (1r files i despres columnes)
	int** files;
	if((files=(int**)malloc((nnodes)*sizeof(int*)))==NULL){
		printf("Error en la reserva de memoria\n");
		return -1;
	}
	
	//FALTA UN FOR AQUI PER A CADA POSICIO DE FILES
	for(i=0;i<nnodes;i++){
		if((files[i]=(int*)malloc((nnodes+1)*sizeof(int)))==NULL){
			printf("Error en la reserva de memoria\n");
			return -1;
		}
	}

	int*** matriu;
	matriu = &files;
	files[0][0]=55;
	
	
	//Omplim la matriu
	printf("aqui si\n");
	printf("%d\n", *matriu[0][0]);
	
	int guardar,j;
	printf("\n");
	for(j=0; j<10;j++){
		printf("i: %d\n",j);
		if(j==6){
			//guardar=i;
			break;
		}
	}
	printf("\n");
	printf("LA J FINAL ES: %d\n", j);

	return 0;
}