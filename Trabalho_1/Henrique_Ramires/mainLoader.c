#include <stdio.h>
#include <stdlib.h>
#include <string.h>		// Para usar strings
#include <dirent.h>

// SOIL é a biblioteca para leitura das imagens
#ifndef __APPLE__
#include "SOIL.h"
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#include "SOIL/SOIL.h"
#endif

// Uma imagem em tons de cinza
typedef struct
{
    int width, height;
    unsigned char* img;
} Img;

// Protótipos
void load(char* name, Img* pic);
float f_media_imagem(unsigned char* vetor_imagem, int const n);
void f_niveis_cinza(unsigned char* vetor_imagem, const int n, unsigned int* histograma_niveis_cinza);
void f_histograma_simplificado(unsigned int* histograma_niveis_cinza, const int n, unsigned int* histograma_simplificado);
float f_desvio_padrao(unsigned int* histograma_niveis_cinza,int const n);
float f_curtose(unsigned int* histograma_niveis_cinza, int const n);
float f_mediana(unsigned int* histograma_niveis_cinza, int const n);
void ordena_crescente(unsigned int* histograma_niveis_cinza, int n);
void f_reducao_tons_cinza(unsigned char* vetor_imagem, const int n, int const QTD_TONS);
void f_convert_para_matriz(unsigned char* vetor_imagem, int nLinhas, int nColunas, unsigned char matriz_imagem[960][1280]);
void f_mco_d10(unsigned char matriz_imagem[960][1280], int const altura, int const largura, unsigned int matriz_imagem_mco_d10[256][256]);
void f_mco_d01(unsigned char matriz_imagem[960][1280], int const altura, int const largura, unsigned int matriz_imagem_mco_d01[256][256]);
void f_mco_d1_1(unsigned char matriz_imagem[960][1280], int const altura, int const largura,  unsigned int matriz_imagem_mco_d1_1[256][256]);
void f_normaliza(unsigned int matriz_imagem_mco[256][256], int const altura, int const largura, float matriz_imagem_mco_normatizada[256][256]);
float f_energia(float matriz_imagem_mco_normatizada[256][256], int const altura, int const largura);
float f_entropia(float matriz_imagem_mco_normatizada[256][256], int const altura, int const largura);
float f_contraste(float matriz_imagem_mco_normatizada[256][256], int const altura, int const largura);
float f_variancia(float matriz_imagem_mco_normatizada[256][256], int const altura, int const largura);
float f_homogeneidade(float matriz_imagem_mco_normatizada[256][256], int const altura, int const largura);
void cabecalho(char* name_arq);
void escreve(char* name_arq, const char* image_name, float media_imagem, float desvio_padrao, float curtose, float mediana,
             float energia_d10, float entropia_d10, float contraste_d10, float variancia_d10, float homogeneidade_d10,
             float energia_d01, float entropia_d01, float contraste_d01, float variancia_d01, float homogeneidade_d01,
             float energia_d1_1, float entropia_d1_1, float contraste_d1_1, float variancia_d1_1, float homogeneidade_d1_1);
void calcula(char* name_arq, char* name, Img* pic);
void gera_csv(char* nome);

// Carrega uma imagem para a struct Img
void load(char* name, Img* pic)
{
    int chan;
    pic->img = (unsigned char*) SOIL_load_image(name, &pic->width, &pic->height, &chan, SOIL_LOAD_AUTO);
    if(!pic->img)
    {
        printf( "SOIL loading error: '%s'\n", SOIL_last_result() );
        exit(1);
    }
    //printf("Load: %d x %d x %d\n", pic->width, pic->height, chan);
    printf("Load: %s\n", name);
}

float f_media_imagem(unsigned char* vetor_imagem, int const n)
{
    unsigned int soma = 0;
    unsigned char* const finalVetor = vetor_imagem + n;
    for(unsigned char* p = vetor_imagem; p<finalVetor; ++p)
    {
        soma = soma + *p;
    }
    return soma/n;
}

void f_niveis_cinza(unsigned char* vetor_imagem, const int n, unsigned int* histograma_niveis_cinza)
{
    int count = 0;
    unsigned char* const finalVetor = vetor_imagem + n;
    for(int i=0; i<256; i++)
    {
        for(unsigned char* p = vetor_imagem; p<finalVetor; ++p)
        {
            if(i == *p)
            {
                count++;
            }
        }
        histograma_niveis_cinza[i] = count;
        count = 0;
    }
}

void f_histograma_simplificado(unsigned int* histograma_niveis_cinza, const int n, unsigned int* histograma_simplificado)  // Nao usei
{
    int cont = 0;
    int soma = 0;

    for(int i=0; i<n; i++)
    {
        soma = soma + *(histograma_niveis_cinza+i);
        if((i+1) % 4 == 0 && i != 0)
        {
            histograma_simplificado[cont] = soma;
            cont++;
            soma = 0;
        }
    }
}

float f_desvio_padrao(unsigned int* histograma_niveis_cinza,int const n)
{
    float media = 0;
    for(int i=0; i<n; i++)
    {
        media = media + histograma_niveis_cinza[i];
    }
    media = media/n;

    float soma = 0;
    for(int j=0; j<n; j++)
    {
        soma = soma + pow((histograma_niveis_cinza[j] - media), 2);
    }
    return sqrt(soma/n);
}

float f_curtose(unsigned int* histograma_niveis_cinza, int const n)
{
    float media = 0;
    for(int m=0; m<n; m++)
    {
        media = media + histograma_niveis_cinza[m];
    }
    media = media/n;

    float result1 = 0;
    for(int i=0; i<n; i++)
    {
        result1 = result1 +  pow(histograma_niveis_cinza[i] - media, 4);
    }
    result1 = result1 / n;

    float result2 = 0;
    for(int j=0; j<n; j++)
    {
        result2 = result2 +  pow(histograma_niveis_cinza[j] - media, 2);
    }
    result2 = pow(result2, 2);
    result2 = result2 / pow(n, 2);

    return result1 / result2;
}

float f_mediana(unsigned int* histograma_niveis_cinza, int const n)
{
    ordena_crescente(histograma_niveis_cinza, 256);
    register int i; /* indexadores com tipo register dao mais ganho de performance no processamento pois sao armazenados em registradores. */
    unsigned int m1, m2;
    float result = 0;

    switch (n % 2)  // Seletor para calculo da mediana.
    {
    case 0: // Faixa de valores (qtd de elem do vetor) e PAR.
        m1 = histograma_niveis_cinza[n/2];
        m2 = histograma_niveis_cinza[(n / 2) + 1];
        return  (float)(m1+m2)/2;
    case 1: // Faixa de valores do vetor e IMPAR.
        m1 = histograma_niveis_cinza[ (int)((n/2) + 1) ];
        return m1;
    }
}

void ordena_crescente(unsigned int* histograma_niveis_cinza, int n)
{
    register int i, j; // indexadores.
    int aux;  // variavel auxiliar.

    for (i = 0; i < (n - 1); i++)
        for (j = i + 1; j < n; j++)
            if (histograma_niveis_cinza[i] > histograma_niveis_cinza[j])
            {
                aux = histograma_niveis_cinza[i];
                histograma_niveis_cinza[i] = histograma_niveis_cinza[j];
                histograma_niveis_cinza[j] = aux;
            }
}

void f_reducao_tons_cinza(unsigned char* vetor_imagem, const int n, int const QTD_TONS)
{
    for(int i=0; i<n; i++)
    {
        vetor_imagem[i] = vetor_imagem[i]/(255/QTD_TONS);
    }
}

void f_convert_para_matriz(unsigned char* vetor_imagem, int nLinhas, int nColunas, unsigned char matriz_imagem[960][1280])
{
    int indice = 0;
    for(int x=0; x<nLinhas; x++)
    {
        for(int y=0; y<nColunas; y++)
        {
            matriz_imagem[x][y] = vetor_imagem[indice];
            indice++;
        }
    }
}

void f_mco_d10(unsigned char matriz_imagem[960][1280], int const altura, int const largura, unsigned int matriz_imagem_mco_d10[256][256])
{
    unsigned char linha, coluna;
    for(int x=1; x<altura; x++)
    {
        for(int y=0; y<largura; y++)
        {
            linha = matriz_imagem[x][y];
            coluna = matriz_imagem[x-1][y];
            matriz_imagem_mco_d10[linha][coluna]++;
        }
    }
}

void f_mco_d01(unsigned char matriz_imagem[960][1280], int const altura, int const largura, unsigned int matriz_imagem_mco_d01[256][256])
{
    unsigned char linha, coluna;
    for(int x=0; x<altura; x++)
    {
        for(int y=0; y<largura-1; y++)
        {
            linha = matriz_imagem[x][y];
            coluna = matriz_imagem[x][y+1];
            matriz_imagem_mco_d01[linha][coluna]++;
        }
    }
}

void f_mco_d1_1(unsigned char matriz_imagem[960][1280], int const altura, int const largura,  unsigned int matriz_imagem_mco_d1_1[256][256])
{
    unsigned char linha, coluna;
    for(int x=1; x<altura; x++)
    {
        for(int y=1; y<largura; y++)
        {
            linha = matriz_imagem[x][y];
            coluna = matriz_imagem[x-1][y-1];
            matriz_imagem_mco_d1_1[linha][coluna]++;
        }
    }
}

void f_normaliza(unsigned int matriz_imagem_mco[256][256], int const altura, int const largura, float matriz_imagem_mco_normatizada[256][256])
{
    unsigned char maior = matriz_imagem_mco[0][0];
    for(int i=0; i<altura; i++)
    {
        for(int j=0; j<largura; j++)
        {
            if(matriz_imagem_mco[i][j] > maior)
            {
                maior = matriz_imagem_mco[i][j];
            }
        }
    }

    for(int k=0; k<altura; k++)
    {
        for(int l=0; l<largura; l++)
        {
            if(maior != 0)
            {
                matriz_imagem_mco_normatizada[k][l] = matriz_imagem_mco[k][l] / maior;
            }
        }
    }
}

float f_energia(float matriz_imagem_mco_normatizada[256][256], int const altura, int const largura)
{
    float result = 0;
    for(int i=0; i<altura; i++)
    {
        for(int j=0; j<largura; j++)
        {
            result = result + pow(matriz_imagem_mco_normatizada[i][j], 2);
        }
    }
    return result;
}

float f_entropia(float matriz_imagem_mco_normatizada[256][256], int const altura, int const largura)
{
    return sqrt(f_energia(matriz_imagem_mco_normatizada, altura, largura));
}

float f_contraste(float matriz_imagem_mco_normatizada[256][256], int const altura, int const largura)
{
    float result = 0;
    for(int i=0; i<altura; i++)
    {
        for(int j=0; j<largura; j++)
        {
            result = result + (matriz_imagem_mco_normatizada[i][j]) * pow(i-j, 2);
        }
    }
    return result;
}

float f_variancia(float matriz_imagem_mco_normatizada[256][256], int const altura, int const largura)
{
    float result = 0;
    for(int i=0; i<altura; i++)
    {
        for(int j=0; j<largura; j++)
        {
            result = result + (matriz_imagem_mco_normatizada[i][j]) * (i-j);
        }
    }
    return result;
}

float f_homogeneidade(float matriz_imagem_mco_normatizada[256][256], int const altura, int const largura)
{
    float result = 0;
    for(int i=0; i<altura; i++)
    {
        for(int j=0; j<largura; j++)
        {
            result = result + matriz_imagem_mco_normatizada[i][j] * (matriz_imagem_mco_normatizada[i][j] / 1 + pow(i-j, 2));
        }
    }
    return result;
}

void cabecalho(char* name_arq)
{
    FILE* arquivo;
    arquivo = fopen(name_arq, "w");
    if(arquivo == NULL)
    {
        printf("Problema ao criar o arquivo!\n");
        return -1;
    }
    fprintf(arquivo, "Nome, Media, Desvio, Curtose, Mediana, Energia(10), Entropia(10), Contraste(10), Variancia(10), Homogeneidade(10), Energia(01), Entropia(01), Contraste(01), Variancia(01), Homogeneidade(01),Energia(1-1), Entropia(1-1), Contraste(1-1), Variancia(1-1), Homogeneidade(1-1), Classe\n");
    fclose(arquivo);
}

void escreve(char* name_arq, const char* image_name, float media_imagem, float desvio_padrao, float curtose, float mediana,
             float energia_d10, float entropia_d10, float contraste_d10, float variancia_d10, float homogeneidade_d10,
             float energia_d01, float entropia_d01, float contraste_d01, float variancia_d01, float homogeneidade_d01,
             float energia_d1_1, float entropia_d1_1, float contraste_d1_1, float variancia_d1_1, float homogeneidade_d1_1)
{

    FILE* arquivo;
    arquivo = fopen(name_arq, "a");
    if(arquivo == NULL)
    {
        printf("Problema ao criar o arquivo!\n");
        return -1;
    }
    char* nome_classe;
    if(image_name[2] == '2' && image_name[3] == '1')
    {
        nome_classe = "Capim";
    }
    else if(image_name[2] == '7' && image_name[3] == '_')
    {
        nome_classe = "Folhas";
    }
    else if(image_name[2] == '1' && image_name[3] == '9')
    {
        nome_classe = "Madeira";
    }
    else if(image_name[2] == '2' && image_name[3] == '2')
    {
        nome_classe = "Piso";
    }
    else if(image_name[2] == '3' && image_name[3] == '_')
    {
        nome_classe = "Rochas";
    }
    else if(image_name[2] == '2' && image_name[3] == '4')
    {
        nome_classe = "Tapete";
    }
    else if(image_name[2] == '8' && image_name[3] == '_')
    {
        nome_classe = "Tijolos";
    }

    fprintf(arquivo, "%s, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %s\n", image_name, media_imagem, desvio_padrao, curtose, mediana,
            energia_d10, entropia_d10, contraste_d10, variancia_d10, homogeneidade_d10,
            energia_d01, entropia_d01, contraste_d01, variancia_d01, homogeneidade_d01,
            energia_d1_1, entropia_d1_1, contraste_d1_1, variancia_d1_1, homogeneidade_d1_1, nome_classe);

    fclose(arquivo);
}

void calcula(char* name_arq, char* name, Img* pic)
{
    // Media
    float media_imagem = f_media_imagem(pic->img, (pic->height*pic->width));

    //-------------------------------------------------------------------------------------
    // Histograma Tons Cinza
    unsigned int* histograma_niveis_cinza;
    if (!(histograma_niveis_cinza = (unsigned int *)malloc(256 * sizeof(unsigned int))))
    {
        printf("Não foi possível alocar o vetor\n");
        exit(0);
    }
    f_niveis_cinza(pic->img, (pic->height*pic->width), histograma_niveis_cinza);

    // Desvio padrao
    float desvio_padrao = f_desvio_padrao(histograma_niveis_cinza, 256);

    // Curtose
    float curtose = f_curtose(histograma_niveis_cinza, 256);

    // Mediana
    float mediana = f_mediana(histograma_niveis_cinza, 256);
    free(histograma_niveis_cinza);

    //-------------------------------------------------------------------------------------
    // Reduz Tons de cinza REMOVER O COMENTARIO
    f_reducao_tons_cinza(pic->img, (pic->height*pic->width), 32);

    //-------------------------------------------------------------------------------------
    // Pega o Vetor de Pixels e Transforma em Uma Matriz
    unsigned char* matriz_imagem;
    if (!(matriz_imagem = (unsigned char *)calloc(960 * 1280, sizeof(unsigned char))))
    {
        printf("Não foi possível alocar a matriz\n");
        exit(0);
    }
    f_convert_para_matriz(pic->img, pic->height, pic->width, matriz_imagem);
    free(pic->img);

    //-------------------------------------------------------------------------------------
    // MCO(1,0)
    unsigned int* matriz_imagem_mco_d10;
    if (!(matriz_imagem_mco_d10 = (unsigned int *)calloc(256 * 256, sizeof(unsigned int))))
    {
        printf("Não foi possível alocar a matriz\n");
        exit(0);
    }
    f_mco_d10(matriz_imagem, 960, 1280, matriz_imagem_mco_d10);

    //-------------------------------------------------------------------------------------
    // MCO(1,0) Normalizada
    float* matriz_imagem_mco_normatizada_d10;
    if (!(matriz_imagem_mco_normatizada_d10 = (float *)calloc(256 * 256, sizeof(float))))
    {
        printf("Não foi possível alocar a matriz\n");
        exit(0);
    }
    f_normaliza(matriz_imagem_mco_d10, 256, 256, matriz_imagem_mco_normatizada_d10);
    free(matriz_imagem_mco_d10);

    //-------------------------------------------------------------------------------------
    //(1,0)
    float energia_d10 = f_energia(matriz_imagem_mco_normatizada_d10, 256, 256);
    float entropia_d10 = f_entropia(matriz_imagem_mco_normatizada_d10, 256, 256);
    float contraste_d10 = f_contraste(matriz_imagem_mco_normatizada_d10, 256, 256);
    float variancia_d10 = f_variancia(matriz_imagem_mco_normatizada_d10, 256, 256);
    float homogeneidade_d10 = f_homogeneidade(matriz_imagem_mco_normatizada_d10, 256, 256);
    free(matriz_imagem_mco_normatizada_d10);

    //-------------------------------------------------------------------------------------
    // MCO(0,1)
    unsigned int* matriz_imagem_mco_d01;
    if (!(matriz_imagem_mco_d01 = (unsigned int *)calloc(256 * 256, sizeof(unsigned int))))
    {
        printf("Não foi possível alocar a matriz\n");
        exit(0);
    }
    f_mco_d01(matriz_imagem, 960, 1280, matriz_imagem_mco_d01);

    //-------------------------------------------------------------------------------------
    // MCO(0,1) Normalizada
    float* matriz_imagem_mco_normatizada_d01;
    if (!(matriz_imagem_mco_normatizada_d01 = (float *)calloc(256 * 256, sizeof(float))))
    {
        printf("Não foi possível alocar a matriz\n");
        exit(0);
    }
    f_normaliza(matriz_imagem_mco_d01, 256, 256, matriz_imagem_mco_normatizada_d01);
    free(matriz_imagem_mco_d01);

    //-------------------------------------------------------------------------------------
    //(0,1)
    float energia_d01 = f_energia(matriz_imagem_mco_normatizada_d01, 256, 256);
    float entropia_d01 = f_entropia(matriz_imagem_mco_normatizada_d01, 256, 256);
    float contraste_d01 = f_contraste(matriz_imagem_mco_normatizada_d01, 256, 256);
    float variancia_d01 = f_variancia(matriz_imagem_mco_normatizada_d01, 256, 256);
    float homogeneidade_d01 = f_homogeneidade(matriz_imagem_mco_normatizada_d01, 256, 256);
    free(matriz_imagem_mco_normatizada_d01);

    //-------------------------------------------------------------------------------------
    // MCO(1,-1)
    unsigned int* matriz_imagem_mco_d1_1;
    if (!(matriz_imagem_mco_d1_1 = (unsigned int *)calloc(256 * 256, sizeof(unsigned int))))
    {
        printf("Não foi possível alocar a matriz\n");
        exit(0);
    }
    f_mco_d1_1(matriz_imagem, 960, 1280, matriz_imagem_mco_d1_1);
    free(matriz_imagem);

    //-------------------------------------------------------------------------------------
    // MCO(1,-1) Normalizada
    float* matriz_imagem_mco_normatizada_d1_1;
    if (!(matriz_imagem_mco_normatizada_d1_1 = (float *)calloc(256 * 256, sizeof(float))))
    {
        printf("Não foi possível alocar a matriz\n");
        exit(0);
    }
    f_normaliza(matriz_imagem_mco_d1_1, 256, 256, matriz_imagem_mco_normatizada_d1_1);
    free(matriz_imagem_mco_d1_1);

    //-------------------------------------------------------------------------------------
    //(1,-1)
    float energia_d1_1 = f_energia(matriz_imagem_mco_normatizada_d1_1, 256, 256);
    float entropia_d1_1 = f_entropia(matriz_imagem_mco_normatizada_d1_1, 256, 256);
    float contraste_d1_1 = f_contraste(matriz_imagem_mco_normatizada_d1_1, 256, 256);
    float variancia_d1_1 = f_variancia(matriz_imagem_mco_normatizada_d1_1, 256, 256);
    float homogeneidade_d1_1 = f_homogeneidade(matriz_imagem_mco_normatizada_d1_1, 256, 256);
    free(matriz_imagem_mco_normatizada_d1_1);

    // Grava o arquivo
    escreve(name_arq, name, media_imagem, desvio_padrao, curtose, mediana,
            energia_d10, entropia_d10, contraste_d10, variancia_d10, homogeneidade_d10,
            energia_d01, entropia_d01, contraste_d01, variancia_d01, homogeneidade_d01,
            energia_d1_1, entropia_d1_1, contraste_d1_1, variancia_d1_1, homogeneidade_d1_1);
}

void gera_csv(char* nome)
{
    char* caminho = NULL;
    if(nome == "Treino.csv")
    {
        caminho = "C:\\Users\\Henrique\\Desktop\\PB\\Avaliacao\\Trabalho_1\\T2_Henrique_Ramires\\Imagens_Treino\\";
    }
    else if(nome == "Teste.csv")
    {
        caminho = "C:\\Users\\Henrique\\Desktop\\PB\\Avaliacao\\Trabalho_1\\T2_Henrique_Ramires\\Imagens_Teste\\";
    }
    cabecalho(nome);
    Img pic;
    FILE* leitura;
    DIR *dir; // Ponteiro para um diretor aberto
    struct dirent *entry; // Coisas no diretório
    dir = opendir(caminho);
    while ( ( entry = readdir(dir) ) != NULL )
    {
        if(entry->d_name[0] != '.')
        {
            char* aux[100] = {};
            strcat(aux, caminho);
            strcat(aux, entry->d_name);
            load(aux, &pic);
            calcula(nome, entry->d_name, &pic);
        }
    }
    closedir(dir);
}

int main(int argc, char** argv)
{
    gera_csv("Treino.csv");
    gera_csv("Teste.csv");
    system("python classificador.py Treino.csv Teste.csv");
}
