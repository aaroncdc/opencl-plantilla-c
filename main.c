/*
* Plantilla básica de OpenCL en C
* 
* Esta plantilla incluye código en C para ejecutar un kernel con OpenCL, y el
* correspondiente código del Kernel. El Kernel se ejecuta a traves de un grupo
* de trabajo en OpenCL, siendo el grupo de trabajo el conjunto de todas las
* unidades computacionales disponibles en el dispositivo especificado. Por
* ejemplo, el conjunto de hilos (threads) y núcleos (cores) del procesador o
* del procesador gráfico.
*
* El kernel de éste ejemplo toma tres parametros: Un array de entrada con un
* set de datos numéricos de tipo coma flotante de precisión simple (float), un
* array de salida con la salida del proceso, y un valor numérico integro con el
* tamaño de ambos sets de datos. El proceso calcula el cuadrado de cada uno de
* los valores incluidos en el set de datos de entrada y coloca cada resultado en
* el array de salida.
*
* El kernel se ejecuta en éste caso dentro de la GPU o iGPU del sistema, siempre
* y cuando el dispositivo lo permita. Esto nos permite realizar tareas computacionales
* con la GPU en lugar de la CPU.
*
* */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __APPLE__
    #include <OpenCL/cl.h>
#else
    #include <CL/cl.h>
#endif

// Tamaño del array de datos usado para el ejemplo.
#define DATA_SIZE (1024)
// Indica que se desea usar la GPU en lugar de la CPU.
#define USEGPU 1

cl_platform_id platform;            // Plataforma
cl_device_id device_id;             // ID del dispositivo 
cl_context context;                 // Contexto computacional
cl_command_queue commands;          // Cola de comandos a computar
cl_program program;                 // Programa
cl_kernel kernel;                   // Kernel
    
cl_mem input;                       // Memoria en el dispositivo reservada para las entradas
cl_mem output;                      // Memoria en el dispositivo reservada para las salidas.

/* Función para cargar un archivo de Kernel en la memoria */
char* loadClKernel(char* filename)
{
    char* data = 0L;
    size_t sz = 0;
    size_t rd = 0;

    FILE* f = fopen(filename, "r");

    if(!f)
        return 0L;

    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    data = (char*) calloc (1,sz);

    if(!data)
    {
        fclose(f);
        return 0L;
    }

    rd = fread(data, sz, 1, f);

    fclose(f);
    return data;
}

/* Rutina principal */
int main()
{
    int err;                            // Código de error de las llamadas a la API.
      
    // Variables usadas para el ejemplo.
    float data[DATA_SIZE];              // Set de datos usados en el ejemplo.
    float results[DATA_SIZE];           // El resultado que devuelve el programa
    unsigned int correct;               // Valores correctos.

    size_t global;                      // global domain size for our calculation
    size_t local;                       // local domain size for our calculation

    // Cargar el kernel en la memoria.
    char* krnSrc = loadClKernel("kernel.cl");
    if(!krnSrc)
    {
        printf("Error cargando el kernel 'kernel.cl'\n");
        return 1;
    }
    
    // Rellenar el set de datos con datos aleatorios.
    int i = 0;
    unsigned int count = DATA_SIZE;
    for(i = 0; i < count; i++)
        data[i] = rand() / (float)RAND_MAX;
    
    // Obtener la ID de la plataforma que estamos usando.
    err = clGetPlatformIDs( 1, &platform, NULL );

    // Conectar con un dispositivo computacional. Esto puede ser una CPU, una GPU...
    err = clGetDeviceIDs(platform, USEGPU ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU , 1, &device_id, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Fallo al crear un dispositivo computacional: %i\n", err);
        return EXIT_FAILURE;
    }
  
    // Crea el contexto del dispositivo.
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
    if (!context)
    {
        printf("Error: Fallo al obtener el contexto del dispositivo.\n");
        return EXIT_FAILURE;
    }

    // Crea la cola de comandos
    commands = clCreateCommandQueue(context, device_id, 0, &err);
    if (!commands)
    {
        printf("Error: Fallo en la creacion de la cola de comandos del dispositivo.\n");
        return EXIT_FAILURE;
    }

    // Crea el programa computacional a traves del kernel que hemos cargado previamente.
    program = clCreateProgramWithSource(context, 1, (const char **) & krnSrc, NULL, &err);
    if (!program)
    {
        printf("Error: Fallo al crear el programa.\n");
        return EXIT_FAILURE;
    }

    // Compila el programa para poder ser usado por el dispositivo.
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        size_t len;
        char buffer[2048];

        printf("-- [ERROR]: Fallo al compilar el programa --\n");
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
        exit(1);
    }

    // Crea el kernel computacional en el programa que vamos a ejecutar en el dispositivo
    kernel = clCreateKernel(program, "square", &err);
    if (!kernel || err != CL_SUCCESS)
    {
        printf("Error: Fallo al crear un kernel computacional.\n");
        exit(1);
    }

    // Crea un array de entrada y otro de salida en la memoria del dispositivo con los valores
    // que vamos a usar para el cálculo a la entrada.
    input = clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(float) * count, NULL, NULL);
    output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * count, NULL, NULL);
    if (!input || !output)
    {
        printf("Error: No se pudo alojar memoria en el dispositivo.\n");
        exit(1);
    }    
    
    // Copia el set de datos en la memoria del dispositivo (en el array de entrada)
    err = clEnqueueWriteBuffer(commands, input, CL_TRUE, 0, sizeof(float) * count, data, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Fallo al escribir en la memoria del dispositivo.\n");
        exit(1);
    }

    /* Enlazamos los argumentos del kernel con los parametros especificados.
    *   input: Array con el set de datos inicial (Entrada)
    *   output: Array con los resultados computados (Salida)
    *   count: Integro con el tamaño del set de datos (Entrada)
    */
    err = 0;
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
    err |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &count);
    if (err != CL_SUCCESS)
    {
        printf("Error: Fallo al enlazar los parametros del kernel: %d\n", err);
        exit(1);
    }

    // Obtener el tamaño máximo del grupo de trabajo (workgroup) para el kernel que
    // se va a ejecutar en el dispositivo. (https://stackoverflow.com/questions/26804153/opencl-work-group-concept)
    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Fallo al obtener información del grupo de trabajo asociado al kernel: %d\n", err);
        exit(1);
    }

    // Ejecuta el kernel usando todos los elementos disponibles en el grupo de trabajo.
    global = count;
    err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
    if (err)
    {
        printf("Error: No se ha podido ejecutar el kernel\n");
        return EXIT_FAILURE;
    }

    // Esperar de forma síncrona a que finalize el proceso antes de leer los resultados.
    clFinish(commands);

    // Obtener los resultados computados en el dispositivo.
    err = clEnqueueReadBuffer( commands, output, CL_TRUE, 0, sizeof(float) * count, results, 0, NULL, NULL );  
    if (err != CL_SUCCESS)
    {
        printf("Error: Fallo al leer la memoria del dispositivo: %d\n", err);
        exit(1);
    }
    
    // Validación de los resultados obtenidos.
    correct = 0;
    for(i = 0; i < count; i++)
    {
        if(results[i] == data[i] * data[i])
            correct++;
    }
    
    // Asumiendo que el tamaño del set (DATA_SIZE) sea 1024, debería
    // de haber 1024 de 1024 resultados correctos.
    printf("Se han computado %d de %d valores de forma correcta\n", correct, count);
    
    // Liberar la memoria usada y finalizar el programa.
    clReleaseMemObject(input);
    clReleaseMemObject(output);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);

    free(krnSrc);

    return 0;
}