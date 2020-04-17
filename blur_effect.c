//Librerias estandar de C
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//Librerias de terceros usadas para manipulacion de imágenes png, jpg, etc.
#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_library/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_library/stb_image_write.h"

//Función para generar el kernel gaussiano
void generate_kernel(int size, double** kernel) 
{   
    //Desviación estándar 
    double sigma = 15.0; 
    
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

    int width, height, channels;

    //Cargamos la imagen obteniendo sus datos
    unsigned char* img = stbi_load(argv[1], &width, &height, &channels, 0);

    if(img == NULL) {
        perror("Error cargando la imagen!\n");
        return EXIT_FAILURE;
    }

    printf("\nancho: %dpx, alto: %dpx, canales: %d\n", width, height, channels);

    int kernel_size;

    //Extracción de tamaño del kernel 
    kernel_size = atoi(argv[3]);
    int mid_size = kernel_size / 2 ;

    //Asignación dinámica de espacio para generar una matriz 
    double** kernel = (double**)malloc(sizeof(double*) * kernel_size);
    for(size_t i = 0; i < kernel_size; ++i) 
        kernel[i] = (double*)malloc(sizeof(double)*kernel_size);
    
    generate_kernel(kernel_size, kernel);

    printf("\nKernel gaussiano usado para el filtro: \n\n");
    for(size_t i = 0; i < kernel_size; ++i) {

        for(size_t j = 0; j < kernel_size; ++j) 
            printf("%f ", kernel[i][j]);
        
        printf("\n");
    }

    size_t img_size = width * height * channels;
    size_t blur_channels = 3;
    size_t blurred_image_size = width * height * blur_channels;


    unsigned char* blurred_img = (unsigned char*)malloc(sizeof(unsigned char) * blurred_image_size);

    size_t sourcePixel = 0;
    size_t endPixel = height*width-1;

    unsigned char* p = img + (sourcePixel*channels);
    unsigned char* b_p = blurred_img + (sourcePixel*blur_channels);

    double valueRed;
    double valueGreen;
    double valueBlue;
    int current_pixel = sourcePixel;
    int target_pixel;
    int pixel_valueBlue;
    int pixel_valueGreen;
    int pixel_valueRed;

    //printf("\nwidth: %dpx, height: %dpx, channels: %d\n", width, height, blur_channels);

    for(; p != img+(endPixel*channels); p += channels, b_p += blur_channels, ++current_pixel) {
        
        valueRed = 0;
        valueGreen = 0;
        valueBlue = 0;

        
            for(int i = -mid_size; i <= mid_size; ++i){
                for(int j = -mid_size; j <= mid_size; ++j){
                    target_pixel = current_pixel + (i*width) + j;

                    pixel_valueRed = target_pixel < 0 || target_pixel > width*height-1 ? 1 : *(img+(target_pixel*channels)+0);
                    pixel_valueGreen = target_pixel < 0 || target_pixel > width*height-1 ? 1 : *(img+(target_pixel*channels)+1);
                    pixel_valueBlue = target_pixel < 0 || target_pixel > width*height-1 ? 1 : *(img+(target_pixel*channels)+2);

                    valueRed += kernel[i+mid_size][j+mid_size] * pixel_valueRed;
                    valueGreen += kernel[i+mid_size][j+mid_size] * pixel_valueGreen;
                    valueBlue += kernel[i+mid_size][j+mid_size] * pixel_valueBlue;
                }
            } 

        *b_p =  (uint8_t)(valueRed);
        *(b_p + 1)=  (uint8_t)(valueGreen);
        *(b_p + 2)=  (uint8_t)(valueBlue);  
    }


    //Escribimos la imágen en formato jpg con el filtro aplicado
    stbi_write_jpg(argv[2], width, height, blur_channels, blurred_img, 100);

    //Liberación de espacio usado por el kernel
    for(size_t i = 0; i < kernel_size; ++i) 
        free(kernel[i]);
    
    free(kernel);

    //Liberación de espacio usado para codificación de la imágen
    stbi_image_free(img);

    return EXIT_SUCCESS;
}