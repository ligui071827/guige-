/***************************************************************************
*
*               Copyright 2006 Mentor Graphics Corporation
*                         All Rights Reserved.
*
* THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS
* THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS
* SUBJECT TO LICENSE TERMS.
*
****************************************************************************/

#include "inflexionui/engine/inc/LcAll.h"
#include "inflexionui/engine/ifxui_engine_porting.h"
#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif


#define TEXT_HEAP_MIN_SIZE 4096
#define TEXT_HEAP_MAX_SIZE 1048576
#define TEXT_HEAP_GROW_BY 4096

LcCEnvironment* LcCEnvironment::s_pEnv = NULL;

extern "C" void* allocateMemory(size_t size) 
{ 
	
	#ifdef IFX_MEMORYTEST_STARTUP

		if (--g_startupTest_allocsUntilFail == 0)
		{
			LC_CLEANUP_THROW(IFX_ERROR_MEMORY);
			return NULL;
		}
	#endif /* IFX_MEMORYTEST_STARTUP */

	#ifdef IFX_MEMORYTEST_DYNAMIC
		g_dynamicTest_currentAllocId++;

		if (EIfxMemoryTestStateNormal == g_dynamicTest_state)
		{
			if (g_dynamicTest_numberOfFailuresToTest <= 0)
			{
				g_dynamicTest_state = EIfxMemoryTestStateComplete;
				IFX_MEMORYTEST_LOG("Test complete.\n");
			}
			else
			{
				if (rand() % g_dynamicTest_chanceOfAllocFail == 0)
				{
					g_dynamicTest_numberOfFailuresToTest--;
					g_dynamicTest_state = EIfxMemoryTestStateBeforeSnapshot;
					g_dynamicTest_snapshotId = g_dynamicTest_currentAllocId;

					IFX_MEMORYTEST_LOG_N("Forcing Alloc #%d to fail...\n", g_dynamicTest_currentAllocId);

					if (LcCEnvironment::get() && LcCEnvironment::get()->getActiveSpace())
					{
						char imagename[256];
						lc_itoa(g_dynamicTest_snapshotId, imagename, 10);
						lc_strcat(imagename, "_Before");
						
						LcCEnvironment::get()->getActiveSpace()->takeScreenshot(imagename);
						g_dynamicTest_state = EIfxMemoryTestStateWaitForCanvasRepaint;
					}

					LC_CLEANUP_THROW(IFX_ERROR_MEMORY);
					return NULL;
				}
			}
		}
	#endif /* IFX_MEMORYTEST_DYNAMIC */

	void *p; 
	
	if (IFXP_Mem_Allocate(IFXP_MEM_ENGINE, (IFX_UINT32)size, &p) == IFX_SUCCESS) 
		return p;  
	LC_CLEANUP_THROW(IFX_ERROR_MEMORY); 
	return NULL; 
}

extern "C" void* allocateMemoryUnsafe(size_t size)
{ 
    void *p;
    if (IFXP_Mem_Allocate(IFXP_MEM_ENGINE, (IFX_UINT32)size, &p) == IFX_SUCCESS)
        return p;
    return NULL;
}

extern "C" void  deallocateMemory(void* ptr)
{
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE, ptr);
}

extern "C" void* allocateMemoryExternal(size_t size)
{
	void *p;
	if (IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL, (IFX_UINT32)size, &p) == IFX_SUCCESS)
		return p;
	return NULL;
}

extern "C" void  deallocateMemoryExternal(void* ptr)
{
	IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, ptr);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void* LcCEnvironment::stringAlloc(int size)
{

#ifdef IFX_MEMORYTEST_STARTUP
	if (--g_startupTest_allocsUntilFail == 0)
	{
		LC_CLEANUP_THROW(IFX_ERROR_MEMORY);
		return NULL;
	}
#endif /* IFX_MEMORYTEST_STARTUP */

#ifdef IFX_MEMORYTEST_DYNAMIC
	g_dynamicTest_currentAllocId++;

	if (EIfxMemoryTestStateNormal == g_dynamicTest_state)
	{
		if (g_dynamicTest_numberOfFailuresToTest < 0)
		{
			g_dynamicTest_state = EIfxMemoryTestStateComplete;
			IFX_MEMORYTEST_LOG("Test complete.\n");
		}
		else
		{
			if (rand() % g_dynamicTest_chanceOfAllocFail == 0)
			{
				g_dynamicTest_numberOfFailuresToTest--;
				g_dynamicTest_state = EIfxMemoryTestStateBeforeSnapshot;
				g_dynamicTest_snapshotId = g_dynamicTest_currentAllocId;

				IFX_MEMORYTEST_LOG_N("Forcing Alloc #%d to fail...\n", g_dynamicTest_currentAllocId);

				if (LcCEnvironment::get() && LcCEnvironment::get()->getActiveSpace())
				{
					char imagename[256];
					lc_itoa(g_dynamicTest_snapshotId, imagename, 10);
					lc_strcat(imagename, "_Before");
					
					LcCEnvironment::get()->getActiveSpace()->takeScreenshot(imagename);
					g_dynamicTest_state = EIfxMemoryTestStateWaitForCanvasRepaint;
				}

				LC_CLEANUP_THROW(IFX_ERROR_MEMORY);
				return NULL;
			}
		}
	}
#endif /* IFX_MEMORYTEST_DYNAMIC */

	void* mem = NULL;

	if (IFX_SUCCESS != IFXP_Mem_Allocate(IFXP_MEM_ENGINE, size, &mem))
		mem = NULL;

	if (mem == NULL)
	{
		LC_CLEANUP_THROW(IFX_ERROR_MEMORY);
	}

	return mem;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCEnvironment::stringFree(void* pPtr)
{
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE, pPtr);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCEnvironment::compressHeap()
{
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcCEnvironment::LcCEnvironment()
{
	m_active = 0;
}
LC_EXPORT LcCEnvironment::~LcCEnvironment()
{
}

/*-------------------------------------------------------------------------*//**
	Returns pointer to single global instance of LcCEnvironment.  Can only
	be called from main UI thread, otherwise unpredictable things could be
	predicted to happen.
*/
LC_EXPORT LcCEnvironment* LcCEnvironment::get()
{
	#ifdef __SYMBIAN32__
		LcCEnvironment* pEnv = (LcCEnvironment*)Dll::Tls();

		return pEnv;
	#else
		return s_pEnv;
	#endif
}

LC_EXPORT void LcCEnvironment::create()
{
	#ifdef __SYMBIAN32__
		LcCEnvironment* pEnv = new LcCEnvironment;
		Dll::SetTls(pEnv);
	#else
		s_pEnv = new LcCEnvironment;
	#endif
}

LC_EXPORT void LcCEnvironment::free()
{
	#ifdef __SYMBIAN32__
		LcCEnvironment* pEnv = (LcCEnvironment*)Dll::Tls();
		delete pEnv;
		Dll::SetTls(NULL);
	#else
		delete s_pEnv;
		s_pEnv = NULL;
	#endif
}

/*-------------------------------------------------------------------------*//**
	Non-inline wrapper so as to not bloat LcTaAuto template expansions
*/
LC_EXPORT void LcCEnvironment::pushCleanupItem(LcCCleanup::TFunction cf, void* obj)
{
	get()->getCleanupStack()->pushItem(cf, obj);
}

/*-------------------------------------------------------------------------*//**
	Non-inline wrapper so as to not bloat LcTaAuto template expansions
*/
LC_EXPORT void LcCEnvironment::popCleanupItem(void* obj)
{
	get()->getCleanupStack()->popItem(obj);
}

