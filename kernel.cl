// Función que calcula el cuadrado de un set de datos
__kernel void square( 
__global float* input,
__global float* output,
const unsigned int count)
{
    // Obtener el tamaño del set de datos (input)
    int i = get_global_id(0);
    // Calcular el cuadrado de cada valor del set
    if(i < count)
        output[i] = input[i] * input[i];
} 