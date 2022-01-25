OPENCL_VERSION=220

all:
	gcc main.c -o opencl -lOpenCL -DCL_TARGET_OPENCL_VERSION=$(OPENCL_VERSION)

debug:
	gcc main.c -g -o opencl -lOpenCL -DCL_TARGET_OPENCL_VERSION=$(OPENCL_VERSION)