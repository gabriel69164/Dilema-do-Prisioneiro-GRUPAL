#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define	AJUDA 1
#define	DEDURA 0
#define NGANG 100 //numero de gangsters
#define NCRI 8 //numero de crimes


/*

	QUANTO MENOS TEMPO DE "PENA" MELHOR PARA O GANGSTER



	G1:AJUDA 		G2: AJUDA ----->  G1->PENA += 1; G2->PENA += 1;
	G1:AJUDA 		G2: DEDURA ----->  G1->PENA +=  5; G2->PENA += 0;
	G1:DEDURA 		G2: AJUDA ------>  G1->PENA += 0; G2->PENA += 5;
	G1: DEDURA		G2: DEDURAM -------> G1->PENA += 3; G2->PENA += 3;

*/
//FUNÇÕES REFERENTES A VETOR PENAS
//compare utilizado no qsort
int cmp(const void* a, const void* b){
	return ( *(int*)a - *(int*)b ); 	
}


//calculo da relevancia. Que é a "classificacao" dos prisioneiros em relacao aos outros
int relevancia(int *pena, int *relevancia, int ngang)
{
	int vetoraux[ngang];
	for (int i = 0; i < ngang; ++i)
	{
		vetoraux[i] = pena[i];
	}
	qsort(vetoraux, ngang, sizeof(int), cmp);
	for (int i = 0; i < ngang; ++i)
	{
		for (int j = 0; j < ngang; ++j)
		{
			if (pena[i] == vetoraux[j])
			{
				relevancia[i] = j;
				break;
			}
		}
	}
}


//porc mutacao eh a funcao que retorna a porcentagem de mutacao do individuo em relacao a sua relevancia
float porc_mutacao(int ngang, int rel){
	return pow((1 + (1/((float)ngang/5) )), (float)rel-((float)ngang/10) );
}


//muda aleatoriamente uma porcentagem da matriz de testemunho do prisioneiro
void mutacao(int*** quad, int* rel, int* penas, int ngang, int ncri){
	
	//este for define o indice dos malhores e coloca em top
	float porc;
	int quant;
	int linha, coluna;
	// tendo o indice dos melhores, mutar todos menos eles
	for(int i=0; i<ngang; i++){
		porc = (porc_mutacao(ngang, rel[i]))/100;
		quant = (porc*ncri);
		for(int j=0;j<quant;j++){
			coluna=rand()%ncri;
			linha=rand()%ngang;
			quad[i][linha][coluna] = (quad[i][linha][coluna]+1)%2;
		}
	}

}


//FUNÇÕES REFRERENTES A QUAD
//copia vetores de uma matriz pra outra
void crossingoveraux(int **pai, int **filho, int ngang, int ncri)
{
	int aux;
	for (int i = 0; i < ngang/2; ++i)
	{
		aux = rand()%ngang;
		for (int j = 0; j < ncri; ++j)
		{
			filho[aux][j] = pai[aux][j]; 
		}
	}
}
//Ele anda pelo grupo de robôs até achar um indivíduo com relevância 0, uma vez que isso ocorre ele fica mandando a matriz
//desse indivíduo com a matriz dos outros indivíduos e joga metade das informações do indivíduo com relevancia 0 nos outros
void crossingover(int ***quad, int *relevancia, int ngang, int ncri)
{
	for (int i = 0; i < ngang; ++i)
	{	
		if (relevancia[i] == 0)
		{
			for (int j = i+1; j < ngang; j++)
			{							 
				if (relevancia[j] != 0)
				{
					crossingoveraux(quad[i], quad[j], ngang, ncri);
				}
			}
		}
	}
}

//cria um vetor randomico
int *vetor_random(int colunas)//ok
{
	int *random = calloc(colunas, sizeof(int));
	for (int i = 0; i < colunas; ++i)
		random[i] = rand()%2;
	return random;
}


//cria uma matriz de tastemunho randomico
int **matriz_random(int ngang, int ncri)//ok
{
	int **tabela_testemunho = calloc(ngang, sizeof(int*));
	for (int i = 0; i < ngang; ++i)
		tabela_testemunho[i] = vetor_random(ncri);
	return tabela_testemunho;
}

//cria os prisioneiros e as matrizes deles
int ***cria_quad(int ngang, int ncri)//ok
{
	int ***quad = calloc(ngang, sizeof(int**));

	for (int i = 0; i < ngang; ++i)
	{
		quad[i] = matriz_random(ngang, ncri);
	}
	return quad;
}

//libera a memoria alocada pelos prisioneiros
void free_quad(int ***quad, int ngang)//ok
{
	for (int i = 0; i < ngang; ++i)
	{
		for (int j = 0; j < ngang; ++j)
		{
			free(quad[i][j]);
		}
		free(quad[i]);
	}
	free(quad);
}

//checka dois testemunhos e rotorna os pontos adicionados as penas de a
int rodada(int a, int b){
	if(a==AJUDA && b==AJUDA){
		return 1;
	}
	else if(a==AJUDA && b==DEDURA){
		return 5;
	}
	else if(a==DEDURA && b==AJUDA){
		return 0;
	}
	else{
		return 3;
	}	
	
}

//adiciona a pena com base na testemunha do prisioneiro e na dos outros
void testemunha(int* a, int*b, int* pena, int ncri,int apos,int bpos){
	for(int i=0;i<ncri;i++){
		pena[apos]+=rodada(a[i],b[i]);
		pena[bpos]+=rodada(b[i],a[i]);
	}
}

//organiza os testemunhos dos prisioneiros uns contra os outros
void julga(int* pena, int*** quad, int ncri, int ngang){
	
	for(int i=0; i<ngang; i++){//avalia individuo i com infdividuo j
		for(int j=i+1; j<ngang; j++){
			//avalia
			testemunha(quad[i][j], quad[j][i], pena, ncri, i, j);
		}
	}
	
}
 //main
int main(int argc, char const *argv[])
{
	srand(time(NULL));	
	int zeron[NGANG], aux_while=-1;
	float media = 0;
	int ***quad = cria_quad(NGANG, NCRI);// quad = quadrilha, na primeira role guarda cada individuo, na segunda role guarda 
			   //as matrizes de açoes dos individuos em relacao aos outros, e na terceira guarda a açao especifica pra cada crime
	int *vetor_penas = calloc(NGANG, sizeof(int));
	int *vetor_relevancia = calloc(NGANG, sizeof(int));



	for (int i = 0; i < NGANG; ++i)//inicia o vetor que conta a quantidade de zeros
	{
		zeron[i] = 0;
	}
	///Printing
	while(aux_while != 0)// enquanto o input nao for zero, roda o programa de novo
	{
		scanf("%d",&aux_while);




		for (int i = 0; i < aux_while; ++i)// roda aux_while geracoes
		{//printf("calculando: %lf por cento\n", ((float)i/(float)aux_while)*100);
			julga(vetor_penas, quad, NCRI, NGANG);
			relevancia(vetor_penas, vetor_relevancia, NGANG);
			mutacao(quad, vetor_relevancia, vetor_penas, NGANG, NCRI);
			crossingover(quad, vetor_relevancia, NGANG, NCRI);
			for (int i = 0; i < NGANG; ++i)
			{
				vetor_penas[i] = 0;
				vetor_relevancia[i] = 0;
			}
		}

		julga(vetor_penas, quad, NCRI, NGANG);
		relevancia(vetor_penas, vetor_relevancia, NGANG);
		
		for (int i = 0; i < NGANG; ++i)// for que conta a quantidade de zeros
		{
			for (int j = 0; j < NGANG; ++j)
			{
				for (int k = 0; k < NCRI; ++k)
				{
					if(quad[i][j][k] == 0)
					{
						zeron[i] += 1;
					}
 				}
			}
		}
		
		for (int i = 0; i < NGANG; ++i)// printando informacoes ordenada com base na relevancia
		{
			for (int j = 0; j < NGANG; ++j)
			{
				if(vetor_relevancia[j]==i){
					printf("gang member: %d  pena: %d  relevancia: %.2d  numero de zeros: %d\n\n", j, vetor_penas[j], vetor_relevancia[j], zeron[j]);
				}
			}
		}

		for (int i = 0; i < NGANG; ++i)//zera o contador de zeros, calcula a media, e zera as penas e a relevancia
		{
			zeron[i] = 0;
			media += vetor_penas[i];
			vetor_penas[i] = 0;
			vetor_relevancia[i] = 0;
		}

		printf("%lf\n", media/NGANG);
		media=0;
	}
	







	free_quad(quad, NGANG);
	free(vetor_penas);
	free(vetor_relevancia);
	return 0;
}
