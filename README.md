# opencl-plantilla-c
 Plantilla para usar OpenCL con C. Incluye el programa principal y un pequeño kernel para OpenCL.

## Prerequisitos
Es necesaria la libreria de OpenCL (libOpenCL). La instalación varía dependiendo de tu sistema operativo o distribución de Linux y de tu hardware (más especificamente, de tu GPU).

· **Ubuntu**: Dependiendo de tu GPU, instala una de las siguientes combinaciones de paquetes: intel-opencl-icd (Para Intel), nvidia-opencl-dev y nvidia-opencl-icd-xxx (Para nVidia, donde 'xxx' es la versión de tus drivers. Por ejemplo, 331), o mesa-opencl-icd (Para AMD). En cualquiera de los casos, deberás instalar también el paquete opencl-headers.
· **Arch Linux**: Sigue las instrucciones en [wiki.archlinux.org](https://wiki.archlinux.org/title/GPGPU)
· **MinGW64**: Instala los paquetes mingw-w64-x86_64-opencl-headers y mingw-w64-x86_64-opencl-icd-git.

## Compilación
Para compilar el programa, sólo tienes que ejecutar el archivo 'makefile' incluido usando GNU MAKE. Este archivo incluye dos reglas para la versión final (all) y depuración (debug). Alternativamente puedes compilarlo a mano usando GCC con el siguiente comando:

    $ gcc main.c -o opencl -lOpenCL -DCL_TARGET_OPENCL_VERSION=$(OPENCL_VERSION)

Donde $(OPENCL_VERSION) es una variable con el número de la versión de OpenCL que quieres usar. Por ejemplo, 220.

## Configruación
Puedes cambiar el valor de la versión de OpenCL a usar en el archivo 'makefile' modificando la siguiente línea:

    OPENCL_VERSION=220

Y reemplazando el valor '220' por el valor de la versión que quieras usar.

## Ejecución
Símplemente ejecuta el archivo binario generado:

    $ ./opencl

Asumiendo que el programa se ejecute de manera correcta, debería de imprimir el siguiente mensaje en la consola:

    Se han computado 1024 de 1024 valores de forma correcta

## Modificación
Para poder adaptar el código a tus necesidades deberás modificar tanto el archivo 'main.c', que contiene el programa principal, como el archivo 'kernel.cl', que contiene el código del kerenel a ejecutar en el dispositivo.

En el archivo 'main.c' deberas reemplazar el código asociado con el kernel de ejemplo con el correspondiente código a usar en tu kernel. A continuación se enumera una lista de líneas de código en main.c asociadas al programa de ejemplo que necesitarás cambiar y/o revisar:

|Linea(s)|Explicación|
|---|---|
|90 a 92|Variables asociadas a los parámetros del kernel.|
|98|Contiene la ruta al archivo del kernel. Puede que necesites cambiarlo.|
|106 a 109|Inicialización del set de datos usado en el ejemplo.|
|160|Cambiar el punto de entrada ('square') por el correspondiente en tu kernel. ([clCreateKernel(3)](https://www.khronos.org/registry/OpenCL/sdk/2.2/docs/man/html/clCreateKernel.html))|
|169 a 170|Se generan buffers en la memoria del dispositivo asociados a los parametros de entrada y salida del kernel.|
|178 a 198|Aquí es donde se copia la entrada de datos y se enlazan los parametros del kernel.|
|222 a 227|Se copia la salida del kernel desde el buffer de salida.|
|230 a 239|Validación de datos. Borralo.|
|242 a 249|Esto es la parte donde se limpia la memoria y se libera el dispositivo. Tendrás que cambiarlo acorde a tus necesidades.|
