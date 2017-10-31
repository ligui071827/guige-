


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

#ifndef LcCSerializeMasterH
#define LcCSerializeMasterH

class ISerializeable
{
public:
#ifdef IFX_SERIALIZATION
		virtual SerializeHandle serialize(LcCSerializeMaster *serializeMaster,bool force=false)=0;
		virtual void deflate(){};
		virtual bool isMenuItemChild()=0;
#endif  /* IFX_SERIALIZATION */
};

class LcTPlacement;
class NdhsCPath;

#ifdef IFX_SERIALIZATION

#define SERIALIZE(var,serializeMaster,cPtr)											\
	serializeMaster->addToBuffer(var,cPtr);

#define SERIALIZE_Placement(var,serializeMaster,cPtr)								\
	serializeMaster->serializePlacement(var,cPtr);

#define DESERIALIZE_Placement(var,serializeMaster,cPtr)								\
	serializeMaster->deSerializePlacement(var,cPtr);

#define DESERIALIZE(var,serializeMaster,cPtr)										\
	serializeMaster->readFromBuffer(var,cPtr);

#define SERIALIZE_Owner(var,serializeMaster,cPtr)									\
	serializeMaster->addToBuffer(var,cPtr);

#define DESERIALIZE_Owner(var,serializeMaster,cPtr,clazz)							\
	{																				\
		SerializeHandle h;															\
		serializeMaster->readFromBuffer(h,cPtr);									\
		if(h!=-1)																	\
		{																			\
			var.set((clazz *)serializeMaster->getPointer(h));						\
			if(!var)																\
				var.set(clazz::loadState(h,serializeMaster));						\
		}																			\
	}

#define SERIALIZE_String(var,serializeMaster,cPtr)									\
	{																				\
		SerializeHandle h = var.serialize(serializeMaster);							\
		serializeMaster->addToBuffer(h,cPtr);										\
	}

#define DESERIALIZE_String(var,serializeMaster,cPtr)								\
	{																				\
		SerializeHandle h;															\
		serializeMaster->readFromBuffer(h,cPtr);									\
		var.deSerialize(serializeMaster->getOffset(h));								\
	}

#define SERIALIZE_Reserve(var,serializeMaster,cPtr)									\
	{																				\
		SerializeHandle h = serializeMaster->reserveHandle(var);					\
		serializeMaster->addToBuffer(h,cPtr);										\
	}

#define DESERIALIZE_Reserve(var,serializeMaster,cPtr,clazz)							\
	{																				\
		SerializeHandle h;															\
		serializeMaster->readFromBuffer(h,cPtr);									\
		var = (clazz*)serializeMaster->getPointer(h);								\
	}

#define SERIALIZE_Ptr(varPtr,serializeMaster,cPtr)									\
	{																				\
		SerializeHandle h = serializeMaster->reserveHandle(varPtr);					\
		serializeMaster->addToBuffer(h,cPtr);										\
	}

#define DESERIALIZE_Ptr(varPtr,serializeMaster,cPtr,clazz)							\
	{																				\
		SerializeHandle h;															\
		serializeMaster->readFromBuffer(h,cPtr);									\
		if(h!=-1)																	\
		{																			\
			varPtr=(clazz *)serializeMaster->getPointer(h);							\
			if(!varPtr)																\
				varPtr=clazz::loadState(h,serializeMaster);							\
		}																			\
	}

#define SERIALIZE_Array(var,serializeMaster,cPtr)									\
	{																				\
		SerializeHandle h = serializeMaster->serialize(var,serializeMaster);		\
		serializeMaster->addToBuffer(h,cPtr);										\
	}

#define SERIALIZE_Array_INT(var,serializeMaster,cPtr)								\
	{																				\
		SerializeHandle h = serializeMaster->serialize(var,serializeMaster);		\
		serializeMaster->addToBuffer(h,cPtr);										\
	}

#define DESERIALIZE_Array(var,serializeMaster,cPtr)									\
	{																				\
		SerializeHandle h;															\
		serializeMaster->readFromBuffer(h,cPtr);									\
		serializeMaster->deSerialize(var,h,serializeMaster);						\
	}
	
#define DESERIALIZE_Array_INT(var,serializeMaster,cPtr)								\
	{																				\
		SerializeHandle h;															\
		serializeMaster->readFromBuffer(h,cPtr);									\
		serializeMaster->deSerialize(var,h,serializeMaster);						\
	}

#define SERIALIZE_MapString(var,serializeMaster,cPtr)								\
	{																				\
		SerializeHandle h = serializeMaster->serialize(var,serializeMaster);		\
		serializeMaster->addToBuffer(h,cPtr);										\
	}

#define DESERIALIZE_MapString(var,serializeMaster,cPtr)								\
	{																				\
		SerializeHandle h;															\
		serializeMaster->readFromBuffer(h,cPtr);									\
		serializeMaster->deSerialize(var,h,serializeMaster);						\
	}

#define SERIALIZE_Observer(var,serializeMaster,cPtr)									\
	{																					\
		SerializeHandle h = serializeMaster->serializeObservers(var,serializeMaster);	\
		serializeMaster->addToBuffer(h,cPtr);											\
	}

#define DESERIALIZE_Observer(var,serializeMaster,cPtr)								\
	{																				\
		SerializeHandle h;															\
		serializeMaster->readFromBuffer(h,cPtr);									\
		serializeMaster->deSerializeObservers(var,h,serializeMaster);				\
	}

typedef LcTmMap<ISerializeable *,SerializeHandle> LcTmPHMap;
typedef LcTmMap<SerializeHandle,ISerializeable *> LcTmHPMap;
typedef LcTmMap<SerializeHandle,int> LcTmHOMap;

class LcCSerializeMaster : public LcCBase
{
private:
	SerializeHandle								m_nextHandle;
	int											m_offset;
	int											m_size;
	volatile int								*m_abortSave;
	LcTByte										*m_currentBuffer;
	LcTmMap<ISerializeable*,SerializeHandle>	m_pointerToHandleMap;
	LcTmMap<SerializeHandle,ISerializeable*>	m_handleToPointerMap;
	LcTmArray<ISerializeable*>					m_deflateRequiredArray;
	LcTmMap<SerializeHandle,int>				m_handleToOffsetMap;
	LcTmMap<SerializeHandle,bool>				m_isSerializedMap;
	LcTmArray<LcTByte *>						m_buffers;
	LcTmArray<int>								m_buffersSize;
	ISerializeable								*m_fullScreenElement;
protected:
					LcCSerializeMaster(int *abortSave){m_abortSave=abortSave;m_nextHandle=0;m_fullScreenElement=NULL;}
public:
	virtual			~LcCSerializeMaster();

	template <class T>
	SerializeHandle	serialize(LcTmArray<T> &arr, LcCSerializeMaster *serializeMaster)
	{				
		int length=arr.size();
		int s = (length+1)*sizeof(IFX_INT32);
		IFX_INT32 * ptr=0,*output=0;
		if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,s,(void **)&output)!=IFX_SUCCESS)
			return -1;
		SerializeHandle  handle = serializeMaster->newHandle();
		ptr=output;
		ptr[0]=length;
		++ptr;
		int i=0;
		for(;i<length;++i)
		{
			SerializeHandle h;
			T obj=arr[i];
			h=serializeMaster->getHandle(obj);
			if(h==-1)
			{				
				h=serializeMaster->reserveHandle(obj);
			}
			*ptr=h;
			++ptr;
		}
		serializeMaster->setData(handle,s,(LcTByte*)output);
		return handle;
	}	
	SerializeHandle	serialize(LcTmArray<int> &arr, LcCSerializeMaster *serializeMaster)
	{				
		int length=arr.size();
		int s = (length+1)*sizeof(IFX_INT32);
		IFX_INT32 * ptr=0,*output=0;
		if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,s,(void **)&output)!=IFX_SUCCESS)
			return -1;
		SerializeHandle  handle = serializeMaster->newHandle();
		ptr=output;
		ptr[0]=length;
		++ptr;
		int i=0;
		for(;i<length;++i)
		{
			*ptr=arr[i];
			++ptr;
		}
		serializeMaster->setData(handle,s,(LcTByte*)output);
		return handle;
	}
	template <class T>
	SerializeHandle	serializeObservers(LcTmArray<T> &arr, LcCSerializeMaster *serializeMaster)
	{				
		int length=arr.size();
		int s = (2*length+1)*sizeof(IFX_INT32);
		IFX_INT32 * ptr=0,*output=0;
		if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,s,(void **)&output)!=IFX_SUCCESS)
			return -1;
		SerializeHandle  handle = serializeMaster->newHandle();
		ptr=output;
		ptr[0]=length;
		++ptr;
		int i=0;
		int type=0;
		ISerializeable* serializeable=NULL;
		for(;i<length;++i)
		{
			SerializeHandle h;
			T obj=arr[i];
			type=-1;
			serializeable=obj->getSerializeAble(type);
			h=serializeMaster->getHandle(serializeable);
			if(h==-1 && type!=-1)
			{
				h=serializeMaster->reserveHandle(serializeable);
			}
			*ptr=h;
			++ptr;
			*ptr=type;
			++ptr;
		}
		serializeMaster->setData(handle,s,(LcTByte*)output);
		return handle;
	}

	void deSerialize(LcTmArray<int> &arr,SerializeHandle handle,LcCSerializeMaster *serializeMaster)
	{		
		IFX_INT32 * ptr=NULL;
		ptr=(IFX_INT32*)serializeMaster->getOffset(handle);
		int length=*ptr;
		++ptr;
		for(int i=0;i<length;++i)
		{
			int val = *ptr;
			arr.push_back(val);			
			++ptr;
		}
	}		

	template <class T>
	void deSerialize(LcTmArray<T*> &arr,SerializeHandle handle,LcCSerializeMaster *serializeMaster)
	{
		typedef T* ObjType;
		IFX_INT32 * ptr=NULL;
		ptr=(IFX_INT32*)serializeMaster->getOffset(handle);
		int length=*ptr;
		++ptr;
		for(int i=0;i<length;++i)
		{
			SerializeHandle h = *ptr;
			if(serializeMaster->hasDeSerialized(h))
			{
				arr.push_back((T*)serializeMaster->getPointer(h));
			}
			else
			{
				ObjType ptr=T::loadState(h,serializeMaster);
				if(ptr!=NULL)
					arr.push_back(ptr);
			}
			++ptr;
		}
	}	

	template <class T>
	void deSerializeObservers(LcTmArray<T*> &arr,SerializeHandle handle,LcCSerializeMaster *serializeMaster)
	{
		typedef T* ObjType;
		IFX_INT32 * ptr=NULL;
		ptr=(IFX_INT32*)serializeMaster->getOffset(handle);
		int length=*ptr;
		++ptr;
		int type=-1;
		for(int i=0;i<length;++i)
		{
			SerializeHandle h = *ptr;
			++ptr;
			type=*ptr;
			++ptr;
			if(h!=-1)
			{
				ObjType p=T::loadState(type,h,serializeMaster);
				if(p!=NULL)
					arr.push_back(p);
			}			
		}
	}	

	template <class T1,class T2>
	SerializeHandle	serialize(LcTmMap<T1,T2> &arr, LcCSerializeMaster *serializeMaster)
	{				
		int length=arr.size();
		int s = (2*length+1)*sizeof(IFX_INT32);
		IFX_INT32 * ptr=0,*output=0;
		if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,s,(void **)&output)!=IFX_SUCCESS)
			return -1;
		SerializeHandle  handle = serializeMaster->newHandle(&arr);
		ptr=output;
		ptr[0]=length;
		++ptr;
		typename LcTmMap<T1,T2>::iterator iter = arr.begin();
		for(;iter!=arr.end();iter++)
		{
			SerializeHandle h;
			T1 obj=(T1)iter->first; 
			h=serializeMaster->getHandle(obj);
			if(h==-1)
				h=serializeMaster->reserveHandle(obj);
			*ptr=h;
			++ptr;

			T2 obj2=iter->second;
			h=serializeMaster->getHandle(obj);
			if(h==-1)
				h=serializeMaster->reserveHandle(obj2);
			*ptr=h;
			++ptr;
		}		
		serializeMaster->setData(handle,s,(LcTByte*)output);
		return handle;
	}

	template <class T1,class T2>
	void deSerialize(LcTmMap<T1*,T2*> &arr,SerializeHandle handle,LcCSerializeMaster *serializeMaster)
	{		
		IFX_INT32 * ptr=NULL;
		T1 obj1;
		T2 obj2;
		ptr=(IFX_INT32*)serializeMaster->getOffset(handle);
		int length=*ptr;
		++ptr;
		for(int i=0;i<length;++i)
		{
			SerializeHandle h = *ptr;
			if(serializeMaster->hasDeSerialized(h))
			{
				obj1=(T1*)serializeMaster->getPointer(h);
			}
			else
			{
				obj1=T1::loadState(h,serializeMaster);				
			}
			++ptr;
			h = *ptr;
			if(serializeMaster->hasDeSerialized(h))
			{
				obj2=(T2*)serializeMaster->getPointer(h);
			}
			else
			{
				obj2=T2::loadState(h,serializeMaster);
			}
			++ptr;
			arr[obj1]=obj2;
		}
	}	

	// specialized for LcTmString

	template <class T2>
	SerializeHandle	serialize(LcTmMap<LcTmString,T2> &arr, LcCSerializeMaster *serializeMaster)
	{				
		int length=arr.size();
		int s = (2*length+1)*sizeof(IFX_INT32);
		IFX_INT32 * ptr=0,*output=0;
		if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,s,(void **)&output)!=IFX_SUCCESS)
			return -1;		
		SerializeHandle  handle = serializeMaster->newHandle();
		ptr=output;
		ptr[0]=length;
		++ptr;
		typename LcTmMap<LcTmString,T2>::iterator iter = arr.begin();
		LcTmString str="";		
		for(;iter!=arr.end();iter++)
		{
			SerializeHandle h;
			str=iter->first; 			
			h=str.serialize(serializeMaster);
			*ptr=h;
			++ptr;

			T2 obj2=iter->second;
			h=serializeMaster->getHandle(obj2);
			if(h==-1)
				h=serializeMaster->reserveHandle(obj2);
			*ptr=h;
			++ptr;
		}		
		serializeMaster->setData(handle,s,(LcTByte*)output);
		return handle;
	}

	template <class T3>
	SerializeHandle	serialize(LcTmMap<LcTmString,LcTmOwner<T3> > &arr, LcCSerializeMaster *serializeMaster)
	{				
		int length=arr.size();
		int s = (2*length+1)*sizeof(IFX_INT32);
		IFX_INT32 * ptr=0,*output=0;
		if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,s,(void **)&output)!=IFX_SUCCESS)
			return -1;
		SerializeHandle  handle = serializeMaster->newHandle();
		ptr=output;
		ptr[0]=length;
		++ptr;
		typename LcTmMap<LcTmString,LcTmOwner<T3> >::iterator iter = arr.begin();
		LcTmString str="";		
		for(;iter!=arr.end();iter++)
		{
			SerializeHandle h;
			str=iter->first; 			
			h=str.serialize(serializeMaster);
			*ptr=h;
			++ptr;

			T3* obj2=iter->second.ptr();
			h=serializeMaster->getHandle(obj2);
			if(h==-1)
				h=serializeMaster->reserveHandle(obj2);
			*ptr=h;
			++ptr;
		}		
		serializeMaster->setData(handle,s,(LcTByte*)output);
		return handle;
	}

	template <class T2>
	void deSerialize(LcTmMap<LcTmString,T2*> &arr,SerializeHandle handle,LcCSerializeMaster *serializeMaster)
	{		
		IFX_INT32 * ptr=NULL;
		LcTmString obj1;
		T2* obj2;
		ptr=(IFX_INT32*)serializeMaster->getOffset(handle);
		int length=*ptr;
		++ptr;
		for(int i=0;i<length;++i)
		{
			SerializeHandle h = *ptr;
			obj1=LcTmString::loadState(h,serializeMaster);				
			++ptr;
			h = *ptr;
			if(serializeMaster->hasDeSerialized(h))
			{
				obj2=(T2*)serializeMaster->getPointer(h);
			}
			else
			{			
				obj2=T2::loadState(h,serializeMaster);
			}
			++ptr;
			arr[obj1]=obj2;
		}
	}	

	template <class T3>
	void deSerialize(LcTmMap<LcTmString,LcTmOwner<T3> > &arr,SerializeHandle handle,LcCSerializeMaster *serializeMaster)
	{		
		IFX_INT32 * ptr=NULL;
		LcTmString obj1;
		LcTaOwner<T3> obj2;
		ptr=(IFX_INT32*)serializeMaster->getOffset(handle);
		int length=*ptr;
		++ptr;
		for(int i=0;i<length;++i)
		{
			SerializeHandle h = *ptr;
			obj1=LcTmString::loadState(h,serializeMaster);				
			++ptr;
			h = *ptr;
			if(serializeMaster->hasDeSerialized(h))
			{
				obj2.set((T3*)serializeMaster->getPointer(h));
			}
			else
			{			
				obj2.set(T3::loadState(h,serializeMaster));
			}
			++ptr;
			arr[obj1]=obj2;
		}
	}	

	template <class Typ> void addToBuffer(Typ &val,void * &buffer)
	{
		if(m_abortSave==NULL || (*m_abortSave)==0)
		{
			memcpy(buffer,&val,sizeof(Typ));		
			buffer=((Typ*)buffer)+1;
		}
	}	
		
	template <class T3>
	void addToBuffer(LcTmOwner<T3> &val,void * &buffer)
	{		
		SerializeHandle h=-1;
		if(val)
			h=this->reserveHandle(val.ptr());
		*((SerializeHandle*)buffer) = h;
		buffer=(SerializeHandle*)buffer+1;
	}
	
	template <class Typ> void readFromBuffer(Typ &val,void * &buffer)
	{
		if(m_abortSave==NULL || (*m_abortSave)==0)
		{
			memcpy(&val,buffer,sizeof(Typ));		
			buffer=((Typ*)buffer)+1;
		}
	}
	
	static LcTaOwner<LcCSerializeMaster> create(int *abortSave);
	ISerializeable * getPointer(SerializeHandle handle);
	void * getOffset(SerializeHandle handle);
	bool   hasDeSerialized(SerializeHandle handle)
	{
		LcTmHPMap::iterator it=m_handleToPointerMap.find(handle);
		return it!=m_handleToPointerMap.end();
	}
	SerializeHandle getHandle(ISerializeable* ptr);
	SerializeHandle newHandle(ISerializeable* ptr);
	inline SerializeHandle newHandle(){return m_nextHandle++;}
	SerializeHandle reserveHandle(ISerializeable* ptr);
	bool 			isSerialized(SerializeHandle handle);
	void			setData(SerializeHandle handle,int size,LcTByte *ptr);
	void			setPointer(SerializeHandle handle,ISerializeable *ptr);
	void			flush();
	bool			load();
	bool			serializeBreadthFirst();
	void			deflate();
	void			serializePlacement(LcTPlacement &val,void * &buffer);	
	void			deSerializePlacement(LcTPlacement &val,void * &buffer);	
	void			requiresDeflate(ISerializeable * serializeable);
	void			setFullScreenElement(ISerializeable* element){ m_fullScreenElement=element;}
	ISerializeable*	getFullScreenElement(){ return m_fullScreenElement;}
};

#endif /* IFX_SERIALIZATION */
#endif /* LcCSerializeMasterH */
