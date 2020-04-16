//Librerias estandar de C
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//Librerias de terceros usadas para manipulacion de imágenes png, jpg, ...

//Función para generar el kernel gaussiano
void generate_kernel(int size, double** kernel) 
{ 
    //Desviación estándar 
    double sigma = 1.0; 
    
    //Suma para normalizar el kernel después 
    double sum = 0.0; 
    
    int mid = size/2;

    //Generar kernel de tamaño size x size usando funcion de densidad de probabilidad de Gauss
    for (int x = -mid; x <= mid; x++) { 
        for (int y = -mid; y <= mid; y++) { 
            kernel[x + mid][y + mid] = exp(-(x * x + y * y) / (2 * sigma * sigma)) / (2 * M_PI * sigma * sigma); 
            sum += kernel[x + mid][y + mid]; 
        } 
    } 
  
    //Normalización del kernel para un efecto mas limpio en el blurring
    for (int i = 0; i < size; ++i) 
        for (int j = 0; j < size; ++j) 
            kernel[i][j] /= sum; 
} 


int main(int argc, char* argv[]) {

    int kernel_size;
    
    //Extracción de tamaño del kernel 
    kernel_size = atoi(argv[3]);


    //Asignación dinámica de espacio para generar una matriz 
    double** kernel = (double**)malloc(sizeof(double*) * kernel_size);
    for(int i = 0; i < kernel_size; ++i) 
        kernel[i] = (double*)malloc(sizeof(double)*kernel_size);
    
    generate_kernel(kernel_size, kernel);

    for(int i = 0; i < kernel_size; ++i) {

        for(int j = 0; j < kernel_size; ++j) 
            printf("%f ", kernel[i][j]);
        
        printf("\n");
    }
    
    //Liberación de espacio usado por el kernel
    for(int i = 0; i < kernel_size; ++i) 
        free(kernel[i]);
    
    free(kernel);

    return EXIT_SUCCESS;
}