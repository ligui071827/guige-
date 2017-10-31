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

#ifdef IFX_SERIALIZATION

#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif

#define IFX_SERIALIZATION_BUFFER_BLOCK	4096

LcTaOwner<LcCSerializeMaster> LcCSerializeMaster::create(int *abortSave)
{
	LcTaOwner<LcCSerializeMaster> ref;
	ref.set(new LcCSerializeMaster(abortSave));
	ref->construct();
	return ref;
}

SerializeHandle LcCSerializeMaster::getHandle(ISerializeable* ptr)
{
	LcTmPHMap::iterator it= m_pointerToHandleMap.find(ptr);
	if(it==m_pointerToHandleMap.end())
		return -1;
	return it->second;
}


/*This will return -1 when handle is already assigned*/
SerializeHandle LcCSerializeMaster::newHandle(ISerializeable* ptr)
{
	if(ptr==NULL || ptr->isMenuItemChild())
		return -1;

	SerializeHandle h= m_nextHandle++;
	LcTmPHMap::iterator it= m_pointerToHandleMap.find(ptr);
	if(it==m_pointerToHandleMap.end())
	{
		m_pointerToHandleMap[ptr]=h;	
	}
	m_handleToPointerMap[h]=ptr;
	return h;
}

SerializeHandle LcCSerializeMaster::reserveHandle(ISerializeable* ptr)
{
	SerializeHandle h=-1;
	if(ptr!=NULL && !ptr->isMenuItemChild())
	{
		h=getHandle(ptr);
		if(h==-1)
			h=newHandle(ptr);
	}
	return h;
}

ISerializeable * LcCSerializeMaster::getPointer(SerializeHandle handle)
{
	if(handle!=-1)
	{		
		LcTmHPMap::iterator it= m_handleToPointerMap.find(handle);
		if(it!=m_handleToPointerMap.end())
			return it->second;
	}
	return NULL;
}

bool LcCSerializeMaster::isSerialized(SerializeHandle handle)
{
	LcTmMap<SerializeHandle,bool>::iterator it= m_isSerializedMap.find(handle);
	if(it==m_isSerializedMap.end())
		return false;
	return it->second;
}

void LcCSerializeMaster::setPointer(SerializeHandle handle,ISerializeable *ptr)
{
	LcTmHPMap::iterator it= m_handleToPointerMap.find(handle);
	if(it!=m_handleToPointerMap.end())
		return;
	m_pointerToHandleMap[ptr]=handle;
	m_handleToPointerMap[handle]=ptr;
}

void LcCSerializeMaster::flush()
{
	LcTmArray<LcTByte *>::iterator iter=m_buffers.begin();
	int bytes_wrote=0;
	IFXP_StateCreate();
	LcTmArray<int>::iterator iterSize=m_buffersSize.begin();	
	for(;iter!=m_buffers.end();iter++)
	{
		int bufferSize=*iterSize;
		if(bufferSize!=0)
		{
			IFXP_StateWrite((char *)(&bufferSize),sizeof(int),&bytes_wrote);
			IFXP_StateWrite((char *)(*iter),bufferSize,&bytes_wrote);
		}
		iterSize++;
	}
	
	for(LcTmHOMap::iterator iter=m_handleToOffsetMap.begin();iter!=m_handleToOffsetMap.end();iter++)
	{
		SerializeHandle h=iter->first;
		int ptr=iter->second;

		IFXP_StateWrite((char *)&h,sizeof(SerializeHandle),&bytes_wrote);
		IFXP_StateWrite((char *)&ptr,sizeof(int),&bytes_wrote);
	}
	int length = m_buffers.size();
	IFXP_StateWrite((char *)&length,sizeof(int),&bytes_wrote);
	length = m_handleToOffsetMap.size();	
	IFXP_StateWrite((char *)&length,sizeof(int),&bytes_wrote);

	IFXP_StateClose();
}

bool LcCSerializeMaster::serializeBreadthFirst()
{
	for(SerializeHandle i=0;(i<m_nextHandle && (m_abortSave==NULL ||(*m_abortSave)==0));++i)
	{
		if(!isSerialized(i))
		{
			if(m_handleToPointerMap[i]!=NULL)
				m_handleToPointerMap[i]->serialize(this);
		}
	}
	if(m_abortSave==NULL ||(*m_abortSave)==0)
	{
		return true;
	}
	return false;
}

void LcCSerializeMaster::deflate()
{
	LcTmArray<ISerializeable*>::iterator iter=m_deflateRequiredArray.begin();
	for(;iter!=m_deflateRequiredArray.end();iter++)
	{
		(*iter)->deflate();
	}
}

void LcCSerializeMaster::serializePlacement(LcTPlacement &val,void * &buffer)
{
	*((SerializeHandle*)buffer)=this->reserveHandle(&val);
	buffer=(SerializeHandle*)buffer+1;
}
void LcCSerializeMaster::deSerializePlacement(LcTPlacement &val,void * &buffer)
{
	SerializeHandle h=-1;
	h=*((SerializeHandle*)buffer);
	buffer=(SerializeHandle*)buffer+1;
	if(h!=-1)
		val.deSerialize(h,this);
}

LcCSerializeMaster::~LcCSerializeMaster()
{
	m_currentBuffer=NULL;
	m_pointerToHandleMap.clear();
	m_handleToPointerMap.clear();
	m_deflateRequiredArray.clear();
	m_handleToOffsetMap.clear();
	m_isSerializedMap.clear();
	LcTmArray<LcTByte *>::iterator iter=m_buffers.begin();
	LcTmArray<int>::iterator iterSize=m_buffersSize.begin();
	for(;iter!=m_buffers.end();iter++)
	{
		LcTByte * buffer=*iter;
		int		  bufferSize=*iterSize;
		if(bufferSize!=0)
			IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,(void *)buffer);
		iterSize++;
	}
}

void LcCSerializeMaster::requiresDeflate(ISerializeable* serializeable)
{
	LcTmArray<ISerializeable*>::iterator iter=m_deflateRequiredArray.begin();
	for(;iter!=m_deflateRequiredArray.end();iter++)
	{
		if((*iter)==serializeable)
		{
			return;
		}
	}
	m_deflateRequiredArray.push_back(serializeable);
}

bool LcCSerializeMaster::load()
{	
	IFX_UINT32 buffersLength=0;
	IFX_UINT32 offsetsLength=0;
	IFX_UINT32 bytesRead=0;
	if(IFXP_StateOpen(&buffersLength,&offsetsLength)==IFX_ERROR)
		return false;
	LcTByte *p=NULL;
	unsigned int bufferSize=0;
	for(IFX_UINT32 i=0;i<buffersLength;++i)
	{
		if(bufferSize==0)
		{
			IFXP_StateRead((char *)&bufferSize,sizeof(int),&bytesRead);		
			if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,bufferSize,(void**)&p)!=IFX_SUCCESS)
				return false;

			IFXP_StateRead((char *)p,bufferSize,&bytesRead);		
			if(bytesRead<bufferSize)
			{
				return false;
			}
		}
		m_buffers.push_back(p);
		m_buffersSize.push_back(bytesRead);
		bytesRead=0;
		p+=IFX_SERIALIZATION_BUFFER_BLOCK;
		bufferSize-=IFX_SERIALIZATION_BUFFER_BLOCK;
	}
	
	
	for(IFX_UINT32 i=0;i<offsetsLength;++i)
	{		
		SerializeHandle h=0;
		int ptr=0;
		IFXP_StateRead((char *)&h,sizeof(SerializeHandle),&bytesRead);		
		IFXP_StateRead((char *)&ptr,sizeof(int),&bytesRead);		
		m_handleToOffsetMap[h]=ptr;
	}
	IFXP_StateClose();
	return true;
}

void * LcCSerializeMaster::getOffset(SerializeHandle handle)
{
	LcTmHOMap::iterator it=m_handleToOffsetMap.find(handle);
	if(it==m_handleToOffsetMap.end())
		return NULL;

	int offset=it->second;
	LcTByte *buffer=m_buffers[offset/IFX_SERIALIZATION_BUFFER_BLOCK];
	void *output = (void*)&buffer[offset%IFX_SERIALIZATION_BUFFER_BLOCK];
	return output;
}


void LcCSerializeMaster::setData(SerializeHandle handle,int s, LcTByte *ptr)
{
	LcTmHOMap::iterator it=m_handleToOffsetMap.find(handle);
	if(it!=m_handleToOffsetMap.end())
		return;

	m_handleToOffsetMap[handle]=m_offset;
	m_isSerializedMap[handle]=true;
	int write=0;
	int bufferOffset=m_offset%IFX_SERIALIZATION_BUFFER_BLOCK;
	int remaining=s;	
	while(remaining>0)
	{
		if(m_offset==m_size && s<=IFX_SERIALIZATION_BUFFER_BLOCK)
		{
			LcTByte *p=NULL;
			if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,IFX_SERIALIZATION_BUFFER_BLOCK,(void**)&p)!=IFX_SUCCESS)
				return;

			memset(p,0,IFX_SERIALIZATION_BUFFER_BLOCK);
			m_buffersSize.push_back(IFX_SERIALIZATION_BUFFER_BLOCK);
			m_buffers.push_back(p);
			m_currentBuffer=p;
			m_size+=IFX_SERIALIZATION_BUFFER_BLOCK;
			bufferOffset=0;
		}
		
		if((m_offset+s)>m_size)
		{
			int allocSize=((s+IFX_SERIALIZATION_BUFFER_BLOCK-1)/IFX_SERIALIZATION_BUFFER_BLOCK)*IFX_SERIALIZATION_BUFFER_BLOCK;	// applying ceil to blockSize;
			LcTByte *p=NULL;
			if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,allocSize,(void**)&p)!=IFX_SUCCESS)
				return;

			memset(p,0,allocSize);
			LcTByte * currentBlock=p;	
			int blockSize=allocSize;
			while(currentBlock<(p+allocSize))
			{
				m_buffers.push_back(currentBlock);	
				m_buffersSize.push_back(blockSize);
				blockSize=0;
				m_currentBuffer=currentBlock;
				currentBlock=currentBlock+IFX_SERIALIZATION_BUFFER_BLOCK;
			}
			memcpy(p,ptr,s);
			m_handleToOffsetMap[handle]=m_size;
			m_offset=m_size+s;
			m_size+=allocSize;
			remaining=0;
		}
		else
		{
			write=remaining<=(IFX_SERIALIZATION_BUFFER_BLOCK-bufferOffset)?remaining:IFX_SERIALIZATION_BUFFER_BLOCK-bufferOffset;
			memcpy(&m_currentBuffer[bufferOffset],ptr,write);
			remaining =remaining-write;
			m_offset+=write;
			bufferOffset+=write;
		}
	}
	
}
#endif /* IFX_SERIALIZATION */
