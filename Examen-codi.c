/*  Francesc Albareda  NIU: 1603751  */

#include <stdio.h>
#include <stdlib.h>

#define MAXARST 20
#define MAXINT 32000


typedef struct aresta{
	unsigned node_arribada;
	double pes_aresta;
}aresta;

typedef struct{
	unsigned narst;
	struct aresta arestes[MAXARST];
	int visitat;
	int anterior;
	double distancia;
}node;

typedef struct elementcua{
	int indexnode;
	struct elementcua* seguent;
}elementcua;

typedef struct{
	elementcua* inici;
	elementcua* final;
}UnaCua;


void arestes_reciproques(node*, node*, int);
void posarencua(UnaCua *, int);
void treuelprimer(UnaCua *);
int buscarcomponents(node*,int ***,int);
int* buscararbres(node*,int ***,int, int);
void trobararbres(node* , int***, int*, int, int);
void calculararbres(int, node*, int***, int);
void encuambprioritat(UnaCua*, int, node*);
void mostracami(int, node*, int, int);
void combinacions(node*, int***, int);


int main(int argc, char const *argv[]){
	
	FILE* dades;
	char c, nom_fitxer[20];
	node* nodes; 
	node* nodes2;
	unsigned nnodes, nlinies=0, narst, i, j;
	int num_components_connexes;
	int* AB;
	

	sscanf(argv[1],"%s;",nom_fitxer);
	if((dades=fopen(nom_fitxer, "r"))==NULL){
		printf("Error en l'obertura del fitxer\n");
		return -1;
	}
	
	while((c=fgetc(dades))!=EOF){
		if(c=='\n'){
			nlinies++;
		}
	}
	nlinies --;
	
	rewind(dades);
	
	fscanf(dades, "NumNodes=%u\n",&nnodes);	
	if((nodes=(node*)malloc(nnodes*sizeof(node)))==NULL){
		printf("Error en la reserva de memoria\n");
		return -1;
	}
	if((nodes2=(node*)malloc(nnodes*sizeof(node)))==NULL){
		printf("Error en la reserva de memoria\n");
		return -1;
	}
	
	for(i=0;i<nnodes;i++){
		nodes[i].narst=0;
		nodes2[i].narst=0;
	}

	int node;
	for(i=0;i<nlinies;i++){
		fscanf(dades,"%u",&node);
		narst=0;
		while((c=fgetc(dades))!='\n'){
			fscanf(dades,"%u",&(nodes[node].arestes[narst].node_arribada));
			nodes2[node].arestes[narst].node_arribada = nodes[node].arestes[narst].node_arribada;
			fgetc(dades);
			fscanf(dades,"%lf",&(nodes[node].arestes[narst].pes_aresta));
			nodes2[node].arestes[narst].pes_aresta = nodes[node].arestes[narst].pes_aresta;
			
			narst++;
		}
		nodes[node].narst = narst;
		nodes2[node].narst = narst;
	}
	
	fclose(dades);
	
	arestes_reciproques(nodes, nodes2, nnodes);


	int** files;
	if((files=(int**)malloc((nnodes)*sizeof(int*)))==NULL){
		printf("Error en la reserva de memoria\n");
		return -1;
	}
	
	for(i=0;i<nnodes;i++){
		if((files[i]=(int*)malloc((nnodes+1)*sizeof(int)))==NULL){
			printf("Error en la reserva de memoria\n");
			return -1;
		}
	}

	int*** matriu;
	matriu = &files;
		
	num_components_connexes = buscarcomponents(nodes2, matriu, nnodes);
	printf("\n\nEl nombre de components feblement connexes d'aquest graf es %d\n\n", num_components_connexes);

	//Imprimim matriu (per comprovar que s'ha fet be)
	for(i=0;i<num_components_connexes;i++){
		printf("Numero d'elements de la component %d: %d\n", i+1, (*matriu)[i][0]);
		for(j=1;j<=((*matriu)[i][0]);j++){
			printf(" %d \n",(*matriu)[i][j]);
		}
		printf("\n");
	}
	
	AB = buscararbres(nodes, matriu, nnodes, num_components_connexes);

	printf("\nVector AB: \n");
	for(i=0;i<num_components_connexes;i++){
		printf("  La posicio %d del vector AB es: %d\n", i+1, AB[i]);
	}
	printf("\n");
	
	trobararbres(nodes, matriu, AB, nnodes, num_components_connexes);
	
	printf("\n");
	
	combinacions(nodes, matriu, num_components_connexes);
	
	return 0;
}

void arestes_reciproques(node* nodes, node* nodes2, int nnodes){
	int i,j,k,nodearribada,reciproc;
	for(i=0;i<nnodes;i++){
		for(j=0;j<nodes[i].narst;j++){
			reciproc=0;
			nodearribada = nodes[i].arestes[j].node_arribada;
			for(k=0;k<nodes2[nodearribada].narst;k++){
				if(nodes2[nodearribada].arestes[k].node_arribada == i){
					reciproc = 1;
					break;
				}
			}
			if(reciproc == 0){		
				nodes2[nodearribada].arestes[nodes2[nodearribada].narst].node_arribada = i;
				nodes2[nodearribada].arestes[nodes2[nodearribada].narst].pes_aresta = nodes[i].arestes[j].pes_aresta;
				nodes2[nodearribada].narst++;
			}
		}
	}
}


void posarencua(UnaCua *Q, int indexnode){

	elementcua *aux;
	aux = (elementcua *)malloc(sizeof(elementcua));
	if(aux==NULL){
		printf("No s'ha pogut reservar la memòria\n");
		exit(-1);
	}
	else{
		aux->indexnode = indexnode;
		aux->seguent = NULL;
		if(Q->inici == NULL){
			Q->inici = aux;
			Q->final = aux;
		}
		else{
			(Q->final)->seguent = aux;
			Q->final = aux; 
		}
	}
}


void treuelprimer(UnaCua *Q){
	elementcua *aux;

	if(Q->inici==NULL){
		printf("No hi ha elements a la cua\n");
		exit(-1);
	}

	else{
		aux = Q->inici;
		Q->inici = Q->inici->seguent;
		free(aux);
	}
}


int buscarcomponents(node* nodes2,int *** matriu ,int nnodes){
	int i, seguir=1, index_inici, num_components=0, num_posicio, indexpare, indexfill;
	
	for(i=0;i<nnodes;i++){
		nodes2[i].visitat = 0;
	}
	
	UnaCua Q;

	index_inici=0;

	Q.inici = NULL;
	Q.final = NULL;


	while(seguir==1){
		nodes2[index_inici].visitat=1;
		posarencua(&Q, index_inici);
		num_posicio = 1;
	
		while(Q.inici != NULL){
			indexpare = Q.inici->indexnode;
			treuelprimer(&Q);
			(*matriu)[num_components][num_posicio] = indexpare;
			num_posicio++;
			for(i=0;i<nodes2[indexpare].narst;i++){
				indexfill = nodes2[indexpare].arestes[i].node_arribada;
				if(nodes2[indexfill].visitat == 0){
					posarencua(&Q,indexfill);
					nodes2[indexfill].visitat = 1;
				}
			}
		}
		
		num_posicio--;
		(*matriu)[num_components][0] = num_posicio;
		
		for(i=0;i<nnodes;i++){
			if(nodes2[i].visitat == 0){
				index_inici = i;
				seguir=1;
				break;
			}
		seguir=0;
		}

		num_components++;
	}

	return num_components;
}


int* buscararbres(node* nodes, int *** matriu , int nnodes, int num_components){
	int i, k, trobat, index_inici=0, indexpare, indexfill, j=0, arrel;  //num_posicio, num_components=0
	int* AB;

	if((AB=(int*)malloc(num_components*sizeof(int)))==NULL){
		printf("Error en la reserva de memoria\n");
		exit(-1);
	}

	UnaCua LaCua;

	for(k=0; k<num_components; k++){
		for(i=1;(i<=((*matriu)[k][0]));i++){
			LaCua.inici = NULL;
			LaCua.final = NULL;
			for(int l=0;l<nnodes;l++){
				nodes[l].visitat = 0;
			}
			index_inici = (*matriu)[k][i];
			nodes[index_inici].visitat = 1;
			posarencua(&LaCua, index_inici);
			while(LaCua.inici != NULL){
				indexpare = LaCua.inici->indexnode;
				
				for(j=0;j<nodes[indexpare].narst;j++){
					indexfill = nodes[indexpare].arestes[j].node_arribada;
					if(nodes[indexfill].visitat == 0){
						nodes[indexfill].visitat = 1;
						posarencua(&LaCua,indexfill);
					}
				}
				treuelprimer(&LaCua);
			}
			
			trobat=1;
			
			for(j=1;(j<=((*matriu)[k][0]));j++){
				if(nodes[((*matriu)[k][j])].visitat != 1){
					trobat=0;
					break;
				}
			}
			if(trobat==1){
				arrel = (*matriu)[k][i];
				break;
			}
		}
		if(trobat==1){
			AB[k] = arrel;
		}
		else{
			AB[k] = -1;
		}
	}
	return AB;
}


void trobararbres(node* nodes, int*** matriu, int* AB, int nnodes, int num_components){  //EstatDij
	int j,k,index_inici;
	for(k=0;k<num_components;k++){
		if(AB[k] == -1){
			printf("\nLa component %d no te cap node del qual en surti un arbre d'expansio\n", k+1);
		}
		else{
			
			index_inici = AB[k];
			
			printf("\nCamins de l'arbre d'expansio de la component %d:   (amb node origen %d)\n", k+1, index_inici);
			
			calculararbres(index_inici, nodes, matriu, k);
			
			for(j=1; j<=((*matriu)[k][0]);j++){
				mostracami(((*matriu)[k][j]), nodes, k, index_inici);
			}
		}
	}
}


void calculararbres(int index_inici, node* nodes, int*** matriu, int k){

	int i, indexpare, indexfill;
	double distancia_proposada;

	for(i=1;i<=((*matriu)[k][0]);i++){
		nodes[((*matriu)[k][i])].visitat = 0;
		nodes[((*matriu)[k][i])].distancia = MAXINT;
		nodes[((*matriu)[k][i])].anterior = -1;
	}

	nodes[index_inici].distancia = 0;
	nodes[index_inici].visitat = 1;
	nodes[index_inici].anterior = MAXINT;
	
	UnaCua cua;
	cua.inici=NULL;
	cua.final=NULL;
	
	encuambprioritat(&cua, index_inici, nodes);
	
	while(cua.inici != NULL){
		
		indexpare = cua.inici->indexnode;
		nodes[indexpare].visitat = 1;
		treuelprimer(&cua);
		
		for(i=0;i<nodes[indexpare].narst;i++){
			indexfill = nodes[indexpare].arestes[i].node_arribada;
			distancia_proposada = nodes[indexpare].distancia + nodes[indexpare].arestes[i].pes_aresta;
			
			if( distancia_proposada < nodes[indexfill].distancia ){
				nodes[indexfill].distancia = distancia_proposada;
				nodes[indexfill].anterior = indexpare;
			
				if(nodes[indexfill].visitat == 0){
					nodes[indexfill].visitat = 1;
					encuambprioritat(&cua, indexfill, nodes);
				}
				
			}
		}
	}
}


void mostracami(int node_final, node* nodes, int k, int index_inici){

    int aux = node_final;
	int* c;
    int elements_cami = 0;
	double distancia=0;

    while(aux != MAXINT){
        aux = nodes[aux].anterior;
        elements_cami++;
    }
	
	if((c=(int*)malloc(elements_cami*sizeof(int))) == NULL){
			printf("Error en la reserva de memòria\n");
			exit(-1);
	}

	aux = node_final;

	for(int u=0;u<elements_cami;u++){
		c[u] = aux;
		aux = nodes[aux].anterior;
	}
	
	
	printf("Cami del node %d al node %d: \n  ", index_inici, node_final);
	
	printf("node %d", c[elements_cami-1]);
	for(int j=elements_cami-2;j>=0;j--){
		
		for(int i=0; i<nodes[c[j+1]].narst; i++){
			if(nodes[c[j+1]].arestes[i].node_arribada == c[j]){
				distancia = nodes[c[j+1]].arestes[i].pes_aresta;
				break;
			}
		}
		
		printf("--(%.2lf)-->node %d", distancia, c[j]);
	}
	printf("\n");
	printf("  Distancia total: %.2lf\n", nodes[node_final].distancia);

}

 
 void encuambprioritat(UnaCua *Q, int indexafegir, node* nodes){

	elementcua * afegir;
	elementcua * anterior;
	
	
	if((afegir = (elementcua*)malloc(sizeof(elementcua))) == NULL){
		printf("Error en la reserva de memòria\n");
		exit(-1);
	}

	elementcua* aux;
	
	afegir->indexnode = indexafegir;
	afegir->seguent = NULL;
	

	if(Q->inici == NULL){
		Q->inici = afegir;
	}
	
	else if(nodes[afegir->indexnode].distancia < nodes[Q->inici->indexnode].distancia){
		aux = Q->inici;
		Q->inici = afegir;
		Q->inici->seguent = aux;

	}
	else{
		anterior = Q->inici;
		while( (anterior->seguent != NULL) && (nodes[anterior->seguent->indexnode].distancia < nodes[afegir->indexnode].distancia) ){
			
			anterior = anterior->seguent;
		}	
		afegir->seguent = anterior->seguent;
		anterior->seguent = afegir;
		
		anterior = anterior->seguent;
		

		while( (anterior->seguent != NULL) && (anterior->seguent->indexnode != afegir->indexnode) ){
		
			anterior = anterior->seguent;
		
		}
		if(anterior->seguent != NULL){
			
			afegir = anterior->seguent;
			anterior->seguent = anterior->seguent->seguent;
			free(afegir);
		}		
	}
}


void combinacions(node* nodes, int*** matriu, int num_components){
	int entrada,sortida,k,indexactual;
	int llista[num_components];
	int contador_llista=0;
		
	for(k=0;k<num_components;k++){
		
		if((*matriu)[k][0] > 1){
		
			printf("\nCOMBINACIONS POSSIBLES DE LA COMPONENT %d:\n", k+1);
			
			for(entrada=1;entrada<=((*matriu)[k][0]);entrada++){
				calculararbres((*matriu)[k][entrada],nodes,matriu,k);
				for(sortida=1;sortida<=((*matriu)[k][0]);sortida++){
					if(entrada==sortida){
						continue;
					}
					else{
						indexactual = (*matriu)[k][sortida];
						while(indexactual != MAXINT  && indexactual != -1){
							if(indexactual == (*matriu)[k][entrada]){	
								break;
							}
							indexactual = nodes[indexactual].anterior;
						}
						if(indexactual == MAXINT || indexactual == -1){
							printf("No existeix cap cami del node %d al node %d\n", (*matriu)[k][entrada], (*matriu)[k][sortida]);
						}
						else{
							mostracami((*matriu)[k][sortida], nodes, k, (*matriu)[k][entrada]);
						}
					}
				}
			}
		}
		else{
			llista[contador_llista] = k;
			contador_llista++;
		}
	}
	printf("\nEls components amb nomes un node (que no tenen arestes) son els seguents:\n");
	for(k=0;k<contador_llista;k++){
		printf(" %d ", llista[k]);
	}
}
