//Librerias estandar de C
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>

//Librerias de terceros usadas para manipulacion de imágenes png, jpg, etc.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_library/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_library/stb_image_write.h"


struct convolution_args {

    size_t sourcePixel;
    size_t endPixel;
    double** kernel;
    unsigned char* img;
    unsigned char* blurred_img;
    size_t channels;
    size_t blur_channels;
    size_t kernel_size;
    size_t width;
    size_t height;
    int thread_id;
    int n_threads;
};

void executeConvolution(struct convolution_args args){

    size_t img_size = args.width * args.height;
    size_t blur_channels = args.blur_channels;
    size_t blurred_image_size = img_size * blur_channels;

    //unsigned char* blurred_img = (unsigned char*)malloc(sizeof(unsigned char) * blurred_image_size);
    unsigned char* blurred_img = args.blurred_img;
    unsigned char* img = args.img;

    size_t sourcePixel = args.sourcePixel;
    size_t endPixel = args.endPixel;

    unsigned char* p = img + (sourcePixel*args.channels);
    unsigned char* b_p = blurred_img + (sourcePixel*blur_channels);

    size_t channels = args.channels;

    double valueRed;
    double valueGreen;
    double valueBlue;

    int current_pixel = sourcePixel;
    int target_pixel;
    int pixel_valueBlue;
    int pixel_valueGreen;
    int pixel_valueRed;

    int mid_size = args.kernel_size/2;

    size_t width = args.width;
    size_t height = args.height;

    double** kernel = args.kernel;

    
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
}
  

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


void *assignWork(void *args) 
{ 
    struct convolution_args * my_args = (struct convolution_args *)args;

    size_t img_size = my_args->width * my_args->height;

    size_t load_work = img_size/(my_args->n_threads);

    my_args->sourcePixel = my_args->thread_id * load_work;
    my_args->endPixel = (my_args->thread_id+1)* load_work - 1;

    printf("\nThread number %d executing from pixel %ld to %ld\n", my_args->thread_id, my_args->sourcePixel, my_args->endPixel);
    executeConvolution(*my_args);

    return NULL; 
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
    kernel_size = atoi(argv[4]);
    size_t mid_size = kernel_size / 2 ;

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

    size_t blurred_image_size = width * height * 3;

    unsigned char* blurred_img = (unsigned char*)malloc(sizeof(unsigned char) * blurred_image_size);

    int n_threads = atoi(argv[3]);

    struct convolution_args args[n_threads];

    pthread_t tid[n_threads];

    struct timeval start, end;

	gettimeofday(&start, NULL);

    for (int i = 0; i < n_threads; i++) {

        args[i].img = img;
        args[i].blurred_img = blurred_img;
        args[i].width = width;
        args[i].height = height;
        args[i].channels = channels;
        args[i].blur_channels = 3;
        args[i].kernel = kernel;
        args[i].kernel_size = kernel_size;
        args[i].n_threads = n_threads;
        args[i].thread_id = i;

        pthread_create(&tid[i], NULL, assignWork, (void *)&args[i]);
    } 

    for (int i = 0; i < n_threads; i++) 
        pthread_join(tid[i], NULL);

    gettimeofday(&end, NULL);
    
    //Escribimos la imágen en formato jpg con el filtro aplicado
    stbi_write_jpg(argv[2], width, height, 3, blurred_img, 100);

    //Liberación de espacio usado por el kernel
    for(size_t i = 0; i < kernel_size; ++i) 
        free(kernel[i]);
    
    free(kernel);

    //Liberación de espacio usado para codificación de la imágen
    stbi_image_free(img);

    long seconds = (end.tv_sec - start.tv_sec);
    long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);

    double seconds_d = (double)micros / pow(10,6);

    //Devolver tiempo de ejecución del programa
    printf("\nTiempo de ejecucion: %f segundos\n", seconds_d);

    return EXIT_SUCCESS;
}