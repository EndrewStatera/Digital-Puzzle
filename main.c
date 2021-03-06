#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Para usar strings
#include <time.h>
#include <math.h>

#ifdef WIN32
#include <windows.h> // includes only in MSWindows not in UNIX
#include "gl/glut.h"
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// SOIL é a biblioteca para leitura das imagens
#include "SOIL.h"

// Um pixel RGB (24 bits)
typedef struct
{
    unsigned char r, g, b;
} RGB;

// Uma imagem RGB
typedef struct
{
    int width, height;
    RGB *img;
} Img;

// Protótipos
void load(char *name, Img *pic);
void valida();
int cmp(const void *elem1, const void *elem2);

// Funções da interface gráfica e OpenGL
void init();
void draw();
void keyboard(unsigned char key, int x, int y);

// Largura e altura da janela
int width, height;

// Identificadores de textura
GLuint tex[3];

// As 3 imagens
Img pic[3];

// Imagem selecionada (0,1,2)
int sel;

// Enums para facilitar o acesso às imagens
#define ORIGEM 0
#define DESEJ 1
#define SAIDA 2

//NOSSOS METODOS======================================================================================
void mapeamentoFor(int tam);
void mapeamentoRandom(int tam);
void inverteImagem(int tam);
int valeTroca(int i, int j, int k);
int valeTrocaCauteloso(int i, int j, int k);

//PARTE DO GERADOR DE NUMEROS ALEATORIOS =============================================================
#define NN 312
#define MM 156
#define MATRIX_A 0xB5026F5AA96619E9ULL
#define UM 0xFFFFFFFF80000000ULL /* Most significant 33 bits */
#define LM 0x7FFFFFFFULL         /* Least significant 31 bits */

/* The array for the state vector */
static unsigned long long mt[NN];
/* mti==NN+1 means mt[NN] is not initialized */
static int mti = NN + 1;

void init_genrand64(unsigned long long seed);
void init_by_array64(unsigned long long init_key[], unsigned long long key_length);
unsigned long long genrand64_int64(void);

//=====================================================================================================

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("transition [origem] [destino]\n");
        printf("Origem é a fonte das cores, destino é a imagem desejada\n");
        exit(1);
    }
    glutInit(&argc, argv);

    // Define do modo de operacao da GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // pic[0] -> imagem com as cores
    // pic[1] -> imagem desejada
    // pic[2] -> resultado do algoritmo

    // Carrega as duas imagens
    load(argv[1], &pic[DESEJ]);
    load(argv[2], &pic[ORIGEM]);

    // A largura e altura da janela são calculadas de acordo com a maior
    // dimensão de cada imagem
    width = pic[ORIGEM].width > pic[DESEJ].width ? pic[ORIGEM].width : pic[DESEJ].width;
    height = pic[ORIGEM].height > pic[DESEJ].height ? pic[ORIGEM].height : pic[DESEJ].height;

    // A largura e altura da imagem de saída são iguais às da imagem desejada (1)
    pic[SAIDA].width = pic[DESEJ].width;
    pic[SAIDA].height = pic[DESEJ].height;
    pic[SAIDA].img = malloc(pic[DESEJ].width * pic[DESEJ].height * 3); // W x H x 3 bytes (RGB)

    // Especifica o tamanho inicial em pixels da janela GLUT
    glutInitWindowSize(width, height);

    // Cria a janela passando como argumento o titulo da mesma
    glutCreateWindow("Quebra-Cabeca digital");

    // Registra a funcao callback de redesenho da janela de visualizacao
    glutDisplayFunc(draw);

    // Registra a funcao callback para tratamento das teclas ASCII
    glutKeyboardFunc(keyboard);

    // Cria texturas em memória a partir dos pixels das imagens
    tex[ORIGEM] = SOIL_create_OGL_texture((unsigned char *)pic[ORIGEM].img, pic[ORIGEM].width, pic[ORIGEM].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    tex[DESEJ] = SOIL_create_OGL_texture((unsigned char *)pic[DESEJ].img, pic[DESEJ].width, pic[DESEJ].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Exibe as dimensões na tela, para conferência
    printf("Origem   : %s %d x %d\n", argv[1], pic[ORIGEM].width, pic[ORIGEM].height);
    printf("Desejada : %s %d x %d\n", argv[2], pic[DESEJ].width, pic[DESEJ].height);
    sel = ORIGEM; // pic1

    // Define a janela de visualizacao 2D
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, width, height, 0.0);
    glMatrixMode(GL_MODELVIEW);

    srand(time(0)); // Inicializa gerador aleatório (se for usar random)

    printf("Processando...\n");

    // Copia imagem de origem na imagem de saída
    // (NUNCA ALTERAR A IMAGEM DE ORIGEM NEM DESEJADA)
    int tam = pic[ORIGEM].width * pic[ORIGEM].height;
    memcpy(pic[SAIDA].img, pic[ORIGEM].img, sizeof(RGB) * tam);

    //
    // Neste ponto, voce deve implementar o algoritmo!
    // (ou chamar funcoes para fazer isso)
    

    init_genrand64(time(0));
    mapeamentoRandom(tam);

    
    // Exemplo de manipulação: inverte as cores na imagem de saída

    // for(int i=0; i<tam; i++) {
    //     pic[SAIDA].img[i].r = 255 - pic[SAIDA].img[i].r;
    //     pic[SAIDA].img[i].g = 255 - pic[SAIDA].img[i].g;
    //     pic[SAIDA].img[i].b = 255 - pic[SAIDA].img[i].b;
    // }
    
    // NÃO ALTERAR A PARTIR DAQUI!

    // Cria textura para a imagem de saída
    tex[SAIDA] = SOIL_create_OGL_texture((unsigned char *)pic[SAIDA].img, pic[SAIDA].width, pic[SAIDA].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    // Grava imagem de saída em out.bmp, para conferência
    SOIL_save_image("out.bmp", SOIL_SAVE_TYPE_BMP, pic[SAIDA].width, pic[SAIDA].height, 3, (const unsigned char *)pic[SAIDA].img);

    // Entra no loop de eventos, não retorna
    glutMainLoop();
}

// Carrega uma imagem para a struct Img
void load(char *name, Img *pic)
{
    int chan;
    pic->img = (RGB *)SOIL_load_image(name, &pic->width, &pic->height, &chan, SOIL_LOAD_RGB);
    if (!pic->img)
    {
        printf("SOIL loading error: '%s'\n", SOIL_last_result());
        exit(1);
    }
    printf("Load: %d x %d x %d\n", pic->width, pic->height, chan);
}

// Verifica se o algoritmo foi aplicado corretamente:
// Ordena os pixels da imagem origem e de saída por R, G e B;
// depois compara uma com a outra: devem ser iguais
void valida()
{
    int ok = 1;
    int size = pic[ORIGEM].width * pic[ORIGEM].height;
    // Aloca memória para os dois arrays
    RGB *aux1 = malloc(size * 3);
    RGB *aux2 = malloc(size * 3);
    // Copia os pixels originais
    memcpy(aux1, pic[ORIGEM].img, size * 3);
    memcpy(aux2, pic[SAIDA].img, size * 3);
    // Mostra primeiros 8 pixels de ambas as imagens
    // antes de ordenar (teste)
    for (int i = 0; i < 8; i++)
        printf("[%02X %02X %02X] ", aux1[i].r, aux1[i].g, aux1[i].b);
    printf("\n");
    for (int i = 0; i < 8; i++)
        printf("[%02X %02X %02X] ", aux2[i].r, aux2[i].g, aux2[i].b);
    printf("\n");
    printf("Validando...\n");
    // Ordena ambos os arrays
    qsort(aux1, size, sizeof(RGB), cmp);
    qsort(aux2, size, sizeof(RGB), cmp);
    // Mostra primeiros 8 pixels de ambas as imagens
    // depois de ordenar
    for (int i = 0; i < 8; i++)
        printf("[%02X %02X %02X] ", aux1[i].r, aux1[i].g, aux1[i].b);
    printf("\n");
    for (int i = 0; i < 8; i++)
        printf("[%02X %02X %02X] ", aux2[i].r, aux2[i].g, aux2[i].b);
    printf("\n");

    FILE *fp;
    fp = fopen("status.txt", "w");

    for (int i = 0; i < size; i++)
    {
        if (aux1[i].r != aux2[i].r ||
            aux1[i].g != aux2[i].g ||
            aux1[i].b != aux2[i].b)
        {
            // Se pelo menos um dos pixels for diferente, o algoritmo foi aplicado incorretamente
            printf("*** INVÁLIDO na posição %d ***: %02X %02X %02X -> %02X %02X %02X\n",
                   i, aux1[i].r, aux1[i].g, aux1[i].b, aux2[i].r, aux2[i].g, aux2[i].b);
            ok = 0;
            fprintf(fp, "*** INVÁLIDO na posição ***");
            break;
        }
    }
    // Libera memória dos arrays ordenados
    free(aux1);
    free(aux2);
    if (ok)
    {
        printf(">>>> TRANSFORMAÇÃO VÁLIDA <<<<<\n");
        fprintf(fp, ">>>> TRANSFORMAÇÃO VÁLIDA <<<<<");
    }
    fclose(fp);
}

// Funcao de comparacao para qsort: ordena por R, G, B (desempate nessa ordem)
int cmp(const void *elem1, const void *elem2)
{
    RGB *ptr1 = (RGB *)elem1;
    RGB *ptr2 = (RGB *)elem2;
    unsigned char r1 = ptr1->r;
    unsigned char r2 = ptr2->r;
    unsigned char g1 = ptr1->g;
    unsigned char g2 = ptr2->g;
    unsigned char b1 = ptr1->b;
    unsigned char b2 = ptr2->b;
    int r = 0;
    if (r1 < r2)
        r = -1;
    else if (r1 > r2)
        r = 1;
    else if (g1 < g2)
        r = -1;
    else if (g1 > g2)
        r = 1;
    else if (b1 < b2)
        r = -1;
    else if (b1 > b2)
        r = 1;
    return r;
}

/*
    Para cada posicao de pixel da imagem desejada, testamos se vale a pena trocar o pixel desta posicao na imagem 
    de saida 50X com um indice aleatoriamente selecionado. O pixel encontrado mais proximo desta cor, deve ser trocado
    no fim do laco, assim passando para o proximo pixel da imagem desejada que desejamos encontrar o pixel mais
    adequado para representa-lo na saida, e repetimos o processo. Enquanto o numero de sucessos(contador) for maior
    que 40000 continuamos o processo.
*/
void mapeamentoRandom(int tam)
{
    int contador = 0;
    do
    {
        contador = 0;
        for (int i = 0; i < tam; i++)
        {
            int pixMaisParecido = i;
            char trocou = 0;

            /* Para j< x
                quanto menor o x -> pro: +rápido  contra: -qualidade
                quanto maior o x -> pro: +qualidade  contra: +lento
                
                
            */
            for (int j = 0; j < 40; j++)
            {
                int indice = genrand64_int64() % tam;

                RGB picParecido = pic[SAIDA].img[indice];
                int valeuTroca = 0;
                valeuTroca = valeTrocaCauteloso(i, pixMaisParecido, indice);
                
                if (valeuTroca)
                {
                    pixMaisParecido = indice;
                }
            }
            if (pixMaisParecido != i)
            {
                unsigned int auxR = pic[SAIDA].img[pixMaisParecido].r;
                unsigned int auxG = pic[SAIDA].img[pixMaisParecido].g;
                unsigned int auxB = pic[SAIDA].img[pixMaisParecido].b;

                pic[SAIDA].img[pixMaisParecido].r = pic[SAIDA].img[i].r;
                pic[SAIDA].img[pixMaisParecido].g = pic[SAIDA].img[i].g;
                pic[SAIDA].img[pixMaisParecido].b = pic[SAIDA].img[i].b;

                pic[SAIDA].img[i].r = auxR;
                pic[SAIDA].img[i].g = auxG;
                pic[SAIDA].img[i].b = auxB;
                contador++;
            }
        }
    } while (contador > 40000);
}

/*
A diferença ou distância entre duas cores é uma métrica de interesse na ciência das cores.
Como a maioria das definições de diferença de cores são distâncias dentro de um espaço de cores, 
o meio padrão de determinar distâncias é a distância euclidiana.
Ao buscar mais sobre a diferença de cores, encontramos essa explicação disponivel em https://en.wikipedia.org/wiki/Color_difference
*/
int valeTrocaCauteloso(int desej, int atual, int teste)
{
    RGB* pixelDesej = &pic[DESEJ].img[desej];
    RGB* pixelDesejTeste = &pic[DESEJ].img[teste];
    RGB* pixelAtualParecido = &pic[SAIDA].img[atual];
    RGB* pixelSaidaTeste = &pic[SAIDA].img[teste];

    unsigned char desejR = pixelDesej->r;
    unsigned char desejG = pixelDesej->g;
    unsigned char desejB = pixelDesej->b;

    unsigned char redAtual = pixelAtualParecido->r;
    unsigned char greenAtual = pixelAtualParecido->g;
    unsigned char blueAtual = pixelAtualParecido->b;

    unsigned char redTeste = pixelSaidaTeste->r;
    unsigned char greenTeste = pixelSaidaTeste->g;
    unsigned char blueTeste = pixelSaidaTeste->b;

    /*
    atualDiferenca => Guarda a diferenca de cada uma das 3 cores que compõe o valor RGB do pixel atualmente
    mais parecido com o pixel desejado. 
    */
    RGB atualDiferenca;
    atualDiferenca.r = abs(redAtual - desejR);
    atualDiferenca.g = abs(greenAtual - desejG);
    atualDiferenca.b = abs(blueAtual - desejB);
    double atDif;
    if ((desejR - atualDiferenca.r) / 2 < 128)
        atDif = sqrt(2 * pow(atualDiferenca.r, 2) + 4 * pow(atualDiferenca.g, 2) + pow(atualDiferenca.b, 2));
    else
        atDif = sqrt(3 * pow(atualDiferenca.r, 2) + 4 * pow(atualDiferenca.g, 2) + 2 * pow(atualDiferenca.b, 2));

    /*
    testeDiferenca => Guarda a diferenca de cada uma das 3 cores que compõe o valor RGB do pixel no indice que
    queremos testar com o pixel desejado. 
    */
    RGB testeDiferenca;
    testeDiferenca.r = abs(redTeste - desejR);
    testeDiferenca.g = abs(greenTeste - desejG);
    testeDiferenca.b = abs(blueTeste - desejB);
    int tesDif;
    if ((desejR - testeDiferenca.r) / 2 < 128)
        tesDif = sqrt(2 * pow(testeDiferenca.r, 2) + 4 * pow(testeDiferenca.g, 2) + pow(testeDiferenca.b, 2));
    else
        tesDif = sqrt(3 * pow(testeDiferenca.r, 2) + 4 * pow(testeDiferenca.g, 2) + 2 * pow(testeDiferenca.b, 2));

    /*
    desejTesteDif => Contém a diferença de cada uma das 3 cores que compõem o RGB da imagem na posicao que 
    desejamos testar na imagem desejada, pelas 3 cores do pixel atualmente mais parecido
    */
    RGB desejTesteDif;
    desejTesteDif.r = abs(redAtual - pixelDesejTeste->r);
    desejTesteDif.g = abs(greenAtual - pixelDesejTeste->g);
    desejTesteDif.b = abs(blueAtual - pixelDesejTeste->b);
    int testeAtualDif;
    if ((desejR - testeDiferenca.r) / 2 < 128)
        testeAtualDif = sqrt(2 * pow(desejTesteDif.r, 2) + 4 * pow(desejTesteDif.g, 2) + pow(desejTesteDif.b, 2));
    else
        testeAtualDif = sqrt(3 * pow(desejTesteDif.r, 2) + 4 * pow(desejTesteDif.g, 2) + 2 * pow(desejTesteDif.b, 2));
    
    /*
        Testamos se vale a pena ser trocado tanto o pixel de teste, quanto o pixel atualmente mais parecido. Para isto
        precisamos da distancia entre pixel de teste e o pixel desejado, da distancia entre o pixel atualmente mais 
        parecido e do pixel desejado, e do pixel atualmente mais parecido com o pixel na posicao de teste da imagem
        desejada. 
        
        Entao, caso o pixel de teste seja mais parecido com o pixel desejado que o pixel atualmente mais parecido 
        com o pixel desejado, vale trocar o pixel de teste. 
        Porem, precisamos saber se vale a pena trocar o pixel atualmente mais parecido com o pixel na posicao de teste
        da imagem desejada, e caso o pixel atualmente mais parecido seja mais próximo deste valor do que o pixel de teste,
        retornamos 1, pois vale fazer a troca.
    */
    if (tesDif < atDif && testeAtualDif < tesDif)
        return 1;

    return 0;
}


// Funções de callback da OpenGL
//
// SÓ ALTERE SE VOCÊ TIVER ABSOLUTA CERTEZA DO QUE ESTÁ FAZENDO!
//

// Gerencia eventos de teclado
void keyboard(unsigned char key, int x, int y)
{
    if (key == 27)
    {
        // ESC: libera memória e finaliza
        free(pic[0].img);
        free(pic[1].img);
        free(pic[2].img);
        exit(1);
    }
    if (key >= '1' && key <= '3')
        // 1-3: seleciona a imagem correspondente (origem, desejada e saída)
        sel = key - '1';
    // V para validar a solução
    if (key == 'v')
        valida();
    glutPostRedisplay();
}

// Callback de redesenho da tela
void draw()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Preto
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Para outras cores, veja exemplos em /etc/X11/rgb.txt

    glColor3ub(255, 255, 255); // branco

    // Ativa a textura corresponde à imagem desejada
    glBindTexture(GL_TEXTURE_2D, tex[sel]);
    // E desenha um retângulo que ocupa toda a tela
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
    glVertex2f(0, 0);

    glTexCoord2f(1, 0);
    glVertex2f(pic[sel].width, 0);

    glTexCoord2f(1, 1);
    glVertex2f(pic[sel].width, pic[sel].height);

    glTexCoord2f(0, 1);
    glVertex2f(0, pic[sel].height);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Exibe a imagem
    glutSwapBuffers();
}

/* initializes mt[NN] with a seed */
void init_genrand64(unsigned long long seed)
{
    mt[0] = seed;
    for (mti = 1; mti < NN; mti++)
        mt[mti] = (6364136223846793005ULL * (mt[mti - 1] ^ (mt[mti - 1] >> 62)) + mti);
}

/* generates a random number on [0, 2^64-1]-interval */
unsigned long long genrand64_int64(void)
{
    int i;
    unsigned long long x;
    static unsigned long long mag01[2] = {0ULL, MATRIX_A};

    if (mti >= NN)
    { /* generate NN words at one time */

        /* if init_genrand64() has not been called, */
        /* a default initial seed is used     */
        if (mti == NN + 1)
            init_genrand64(5489ULL);

        for (i = 0; i < NN - MM; i++)
        {
            x = (mt[i] & UM) | (mt[i + 1] & LM);
            mt[i] = mt[i + MM] ^ (x >> 1) ^ mag01[(int)(x & 1ULL)];
        }
        for (; i < NN - 1; i++)
        {
            x = (mt[i] & UM) | (mt[i + 1] & LM);
            mt[i] = mt[i + (MM - NN)] ^ (x >> 1) ^ mag01[(int)(x & 1ULL)];
        }
        x = (mt[NN - 1] & UM) | (mt[0] & LM);
        mt[NN - 1] = mt[MM - 1] ^ (x >> 1) ^ mag01[(int)(x & 1ULL)];

        mti = 0;
    }

    x = mt[mti++];

    x ^= (x >> 29) & 0x5555555555555555ULL;
    x ^= (x << 17) & 0x71D67FFFEDA60000ULL;
    x ^= (x << 37) & 0xFFF7EEE000000000ULL;
    x ^= (x >> 43);

    return x;
}