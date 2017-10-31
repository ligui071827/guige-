/* Inflexion UI Engine includes. */
#include    "inflexionui/porting/inc/ifxui_porting_linux.h"
#include    "inflexionui/engine/ifxui_engine_porting.h"

void *ifx_malloc(int s)
{
	void *ptr = 0;
	if (IFXP_Mem_Allocate(IFXP_MEM_ENGINE, s + sizeof(int), &ptr) != IFX_SUCCESS)
	{
		ptr = 0;
	}
	else
	{
		*((int *)ptr) = s;
		ptr = ((int *)ptr) + 1;
	}
	return ptr;
}

void *ifx_realloc(void *__ptr, size_t __size)
{
	void *ptr = 0;
	int pre_size = 0;
	if (__ptr != 0)
	{
		pre_size = *(((int *)__ptr) - 1);
	}
	if (IFXP_Mem_Allocate(IFXP_MEM_ENGINE, __size + sizeof(int), &ptr) != IFX_SUCCESS)
	{
		ptr = 0;
	}
	else
	{
		*((int *)ptr) = __size;
		ptr = ((int *)ptr) + 1;
		memset(ptr, 0, __size);

		if (pre_size != 0)
		{
			memcpy(ptr, __ptr, pre_size);
			ifx_free(__ptr);
		}
	}
	return ptr;
}

void *mycalloc(size_t __nmemb, size_t __size)
{
	void *ptr = 0;
	if (IFXP_Mem_Allocate(IFXP_MEM_ENGINE, __nmemb * __size + sizeof(int), &ptr) != IFX_SUCCESS)
	{
		ptr = 0;
	}
	else
	{
		*((int *)ptr) = __size;
		ptr = ((int *)ptr) + 1;
		memset(ptr, 0, __nmemb * __size);
	}
	return ptr;
}

void ifx_free(void *ptr)
{
	if (ptr == 0)
		return;

	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE, (((int *)ptr) - 1));
}
