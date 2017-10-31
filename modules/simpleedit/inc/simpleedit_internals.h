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

#ifndef SimpleEditInternalsH
#define SimpleEditInternalsH

#ifdef __cplusplus
extern "C" {
#endif

/* tIFXI_SimpleEdit is the base for the EDITBOX data structure */
typedef struct tSimpleEdit 
{
	IFX_WCHAR*			pBuffer;
	IFX_INT32			maxLength;
	IFX_HMENUID			hMenuId;
	IFX_INT32			item;
	IFX_WCHAR*			szFieldName;
	IFX_INT32			caretPosition;
	IFX_INT32			style;
	IFX_INT32			inputMode;
	IFX_INT32			specialChars;
	IFX_HBUFFERID       bufferID;
}SIMPLE_EDIT;


typedef struct tSimpleEditSession
{
	SIMPLE_EDIT*		pCurrentElement;
    IFX_HMODULEID       hModID;
	IFX_INT32		    lastKey;
}SIMPLE_EDIT_SESSION;

/* Type of keys from the input device */
enum tagKEY_TYPE
{
    TEXT_KEY = 1,
    CONTROL_KEY
};

/* Edit Box constraint Bitfields */
#define	SIMPLE_EDIT_STYLE_PASSWORD         	0x00000001UL // Bit 0
#define	SIMPLE_EDIT_STYLE_PHONE            	0x00000002UL // Bit 1
#define	SIMPLE_EDIT_STYLE_TEXT             	0x00000004UL // Bit 2
#define	SIMPLE_EDIT_STYLE_NUMERIC          	0x00000008UL // Bit 3

#define	SIMPLE_EDIT_INPUT_AUTOCAPWORD      	0x00000010UL // Bit 4
#define	SIMPLE_EDIT_INPUT_AUTOCAPSENTENCE  	0x00000020UL // Bit 5
#define	SIMPLE_EDIT_INPUT_LOWERCASE        	0x00000040UL // Bit 6
#define	SIMPLE_EDIT_INPUT_UPPERCASE        	0x00000080UL // Bit 7
#define	SIMPLE_EDIT_INPUT_NUMERIC			0x00000100UL // Bit 8

IFX_RETURN_STATUS	SimpleEdit_CycleInputMode(SIMPLE_EDIT_SESSION*	pSession,
                  	                          SIMPLE_EDIT*			pElement);

IFX_INT32 SimpleEdit_CheckSpecialChar(IFX_INT32 key);

#ifdef __cplusplus
}
#endif
#endif /* SimpleEditInternalsH */
