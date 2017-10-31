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

#include "inflexionui/engine/ifxui_defs.h"

#include "inflexionui/engine/ifxui_module_integration.h"
#include "inc/simpleedit_internals.h"
#include "inc/simpleedit_multitap.h"
#include "inc/simpleedit_platform.h"

#define KEY_EXPIRE_TIME		350

/* Backspace key to remove a character from the buffer */
#define BACKSPACE_KEY          IFX_KEY_CODE_SELECT

/* Key used to cycle the input mode */
#define INPUT_MODE_KEY_CHANGE  IFX_KEY_CODE_HASH

static IFX_INT32	SimpleEdit_toLower(					IFX_INT32		key);

static IFX_INT32	SimpleEdit_toUpper(					IFX_INT32		key);

static IFX_INT32	SimpleEdit_TextKeyCaseHandler(		SIMPLE_EDIT*	pElement,
														IFX_INT32		key);

static void	SimpleEdit_MultiTapTextKeyConvertor(		IFX_INT32*		_key,
														IFX_INT32*		multiTap,
														IFX_INT32*		currentKey,
														IFX_INT32*		recommendedKey,
														IFX_INT32*		unknownKey);

static void	SimpleEdit_MultiTapPhoneKeyConvertor(		IFX_INT32*		_key,
														IFX_INT32*		multiTap,
														IFX_INT32*		currentKey,
														IFX_INT32*		recommendedKey,
														IFX_INT32*		unknownKey);

static IFX_INT32	SimpleEdit_MultiTapKeyConvertor(	SIMPLE_EDIT_SESSION* pSession,
														SIMPLE_EDIT*	pElement,
														IFX_INT32		key);

static IFX_RETURN_STATUS	SimpleEdit_NavigationKey(	SIMPLE_EDIT*	pElement,
														IFX_INT32		key,
														IFX_INT32*		pConsumed);

static IFX_RETURN_STATUS	SimpleEdit_AppendKey(		SIMPLE_EDIT*	pElement,
														IFX_INT32		key,
														IFX_INT32*		pConsumed);
														
static IFX_RETURN_STATUS	SimpleEdit_DeleteKey(		SIMPLE_EDIT*	pElement,
														IFX_INT32		key,
														IFX_INT32*		pConsumed);

static IFX_RETURN_STATUS	SimpleEdit_ControlKey(		SIMPLE_EDIT_SESSION* pSession,
														SIMPLE_EDIT*	pElement,
														IFX_INT32		key,
														IFX_INT32*		pConsumed,
														IFX_INT32*		pCallback);

/*-------------------------------------------------------------------------*//**

*/
static IFX_INT32 SimpleEdit_toLower(IFX_INT32 key)
{
	if (key >= 'A' && key <= 'Z')
	{
		key += ('a' - 'A');
	}

	return key;
}

/*-------------------------------------------------------------------------*//**

*/
static IFX_INT32 SimpleEdit_toUpper(IFX_INT32 key)
{
	if (key >= 'a' && key <= 'z')
	{
		key -= ('a' - 'A');
	}

	return key;
}

/*-------------------------------------------------------------------------*//**

*/
static IFX_INT32 SimpleEdit_TextKeyCaseHandler(	SIMPLE_EDIT*	pElement,
												IFX_INT32		key)
{
	IFX_INT32 inputMode	= pElement->inputMode;
	IFX_WCHAR previous  = L'\0';

	if (inputMode & SIMPLE_EDIT_INPUT_AUTOCAPWORD)
	{
		if (pElement->caretPosition > 0)
		{
			previous = pElement->pBuffer[pElement->caretPosition - 1];
		}

		if (previous == 0 || previous == ' ' || previous == '.')
		{
			key = SimpleEdit_toUpper(key);
		}
	}
	else if (inputMode & SIMPLE_EDIT_INPUT_AUTOCAPSENTENCE)
	{
		IFX_INT32 pos = pElement->caretPosition;

		/* Consume all the spaces */
		while (pos > 0)
		{
			if(pElement->pBuffer[pos - 1] != ' ')
			{
				break;
			}

			pos--;
		}

		/* Get the word before space */
		if (pos > 0)
		{
			previous = pElement->pBuffer[pos - 1];
		}

		/* If first word of sentence */
		if (previous == 0 || previous == '.')
		{
			key = SimpleEdit_toUpper(key);
		}
	}
	else if (inputMode & SIMPLE_EDIT_INPUT_LOWERCASE)
	{
		key = SimpleEdit_toLower(key);
	}
	else if (inputMode & SIMPLE_EDIT_INPUT_UPPERCASE)
	{
		key = SimpleEdit_toUpper(key);
	}

	return key;
}

/*-------------------------------------------------------------------------*//**

*/
static void SimpleEdit_MultiTapTextKeyConvertor(	IFX_INT32*	_key,
													IFX_INT32*	multiTap,
													IFX_INT32*	currentKey,
													IFX_INT32*	recommendedKey,
													IFX_INT32*	unknownKey)
{

	// Check the ASCII value of the key pressed
    switch (*_key)
	{
		case '1':
		{
			switch (*currentKey)
			{
				case '|':
				{
					*recommendedKey = '.';
					break;
				}
				case '.':
				{
					*recommendedKey = ',';
					break;
				}
				case ',':
				{
					*recommendedKey = '1';
					break;
				}
				case '1':
				{
					*recommendedKey = '`';
					break;
				}
				case '`':
				{
					*recommendedKey = '~';
					break;
				}
				case '~':
				{
					*recommendedKey = '!';
					break;
				}
				case '!':
				{
					*recommendedKey = '@';
					break;
				}
				case '@':
				{
					*recommendedKey = '#';
					break;
				}
				case '#':
				{
					*recommendedKey = '$';
					break;
				}
				case '$':
				{
					*recommendedKey = '%';
					break;
				}
				case '%':
				{
					*recommendedKey = '^';
					break;
				}
				case '^':
				{
					*recommendedKey = '&';
					break;
				}
				case '&':
				{
					*recommendedKey = '*';
					break;
				}
				case '*':
				{
					*recommendedKey = '(';
					break;
				}
				case '(':
				{
					*recommendedKey = ')';
					break;
				}
				case ')':
				{
					*recommendedKey = '-';
					break;
				}
				case '-':
				{
					*recommendedKey = '_';
					break;
				}
				case '_':
				{
					*recommendedKey = '=';
					break;
				}
				case '=':
				{
					*recommendedKey = '+';
					break;
				}
				case '+':
				{
					*recommendedKey = '[';
					break;
				}
    			case '[':
                {
					*recommendedKey = ']';
					break;
				}
				case ']':
				{
					*recommendedKey = '{';
					break;
				}
				case '{':
				{
					*recommendedKey = '}';
					break;
				}
				case '}':
				{
					*recommendedKey = '<';
					break;
				}
				case '<':
				{
					*recommendedKey = '>';
					break;
				}
				case '>':
				{
					*recommendedKey = '/';
					break;
				}
				case '/':
				{
					*recommendedKey = '?';
					break;
				}
				case '?':
				{
					*recommendedKey = ':';
					break;
				}
				case ':':
				{
					*recommendedKey = ';';
					break;
				}
				case ';':
				{
					*recommendedKey = '"';
					break;
				}
				case '"':
				{
					*recommendedKey = '\'';
					break;
				}
				case '\'':
				{
					*recommendedKey = '\\';
					break;
				}
				case '\\':
				{
					*recommendedKey = '|';
					break;
				}
				default:
				{
					*recommendedKey = '.';
					*multiTap = IFX_FALSE;
					break;
				}
			}

			break;
		}
		case '2':
		{
			switch (*currentKey)
			{
				case '2':
				{
					*recommendedKey = 'a';
					break;
				}
				case 'a':
				{
					*recommendedKey = 'b';
					break;
				}
				case 'b':
				{
					*recommendedKey = 'c';
					break;
				}
				case 'c':
				{
					*recommendedKey = '2';
					break;
				}
				default:
				{
					*recommendedKey = 'a';
					*multiTap = IFX_FALSE;
					break;
				}
			}

			break;
		}
		case '3':
		{
			switch (*currentKey)
			{
				case '3':
				{
					*recommendedKey = 'd';
					break;
				}
				case 'd':
				{
					*recommendedKey = 'e';
					break;
				}
				case 'e':
				{
					*recommendedKey = 'f';
					break;
				}
				case 'f':
				{
					*recommendedKey = '3';
					break;
				}
				default:
				{
					*recommendedKey = 'd';
					*multiTap = IFX_FALSE;
					break;
				}
			}

			break;
		}
		case '4':
		{
			switch (*currentKey)
			{
				case '4':
				{
					*recommendedKey = 'g';
					break;
				}
				case 'g':
				{
					*recommendedKey = 'h';
					break;
				}
				case 'h':
				{
					*recommendedKey = 'i';
					break;
				}
				case 'i':
				{
					*recommendedKey = '4';
					break;
				}
				default:
				{
					*recommendedKey = 'g';
					*multiTap = IFX_FALSE;
					break;
				}
			}

			break;
		}
		case '5':
		{
			switch (*currentKey)
			{
				case '5':
				{
					*recommendedKey = 'j';
					break;
				}
				case 'j':
				{
					*recommendedKey = 'k';
					break;
				}
				case 'k':
				{
					*recommendedKey = 'l';
					break;
				}
				case 'l':
				{
					*recommendedKey = '5';
					break;
				}
				default:
				{
					*recommendedKey = 'j';
					*multiTap = IFX_FALSE;
					break;
				}
			}

			break;
		}
		case '6':
		{
			switch (*currentKey)
			{
				case '6':
				{
					*recommendedKey = 'm';
					break;
				}
				case 'm':
				{
					*recommendedKey = 'n';
					break;
				}
				case 'n':
				{
					*recommendedKey = 'o';
					break;
				}
				case 'o':
				{
					*recommendedKey = '6';
					break;
				}
				default:
				{
					*recommendedKey = 'm';
					*multiTap = IFX_FALSE;
					break;
				}
			}

			break;
		}
		case '7':
		{
			switch (*currentKey)
			{
				case '7':
				{
					*recommendedKey = 'p';
					break;
				}
				case 'p':
				{
					*recommendedKey = 'q';
					break;
				}
				case 'q':
				{
					*recommendedKey = 'r';
					break;
				}
				case 'r':
				{
					*recommendedKey = 's';
					break;
				}
				case 's':
				{
					*recommendedKey = '7';
					break;
				}
				default:
				{
					*recommendedKey = 'p';
					*multiTap = IFX_FALSE;
					break;
				}
			}

			break;
		}
		case '8':
		{
			switch (*currentKey)
			{
				case '8':
				{
					*recommendedKey = 't';
					break;
				}
				case 't':
				{
					*recommendedKey = 'u';
					break;
				}
				case 'u':
				{
					*recommendedKey = 'v';
					break;
				}
				case 'v':
				{
					*recommendedKey = '8';
					break;
				}
				default:
				{
					*recommendedKey = 't';
					*multiTap = IFX_FALSE;
					break;
				}
			}

			break;
		}
		case '9':
		{
			switch (*currentKey)
			{
				case '9':
				{
					*recommendedKey = 'w';
					break;
				}
				case 'w':
				{
					*recommendedKey = 'x';
					break;
				}
				case 'x':
				{
					*recommendedKey = 'y';
					break;
				}
				case 'y':
				{
					*recommendedKey = 'z';
					break;
				}
				case 'z':
				{
					*recommendedKey = '9';
					break;
				}
				default:
				{
					*recommendedKey = 'w';
					*multiTap = IFX_FALSE;
					break;
				}
			}

			break;
		}
		case '0':
		{
			switch (*currentKey)
			{
				case '0':
				{
					*recommendedKey = ' ';
					break;
				}
				case ' ':
				{
					*recommendedKey = '0';
					break;
				}
				default:
				{
					*recommendedKey = ' ';
					*multiTap = IFX_FALSE;
					break;
				}
			}

			break;
		}
		case IFX_KEY_CODE_ASTERISK:
		{
			*recommendedKey = '*';
			*multiTap	= IFX_FALSE;
			break;
		}
		default:
		{
			*unknownKey	= IFX_TRUE;
			*multiTap	= IFX_FALSE;
			break;
		}
	}

}

/*-------------------------------------------------------------------------*//**

*/
static void SimpleEdit_MultiTapPhoneKeyConvertor(	IFX_INT32*	_key,
													IFX_INT32*	multiTap,
													IFX_INT32*	currentKey,
													IFX_INT32*	recommendedKey,
													IFX_INT32*	unknownKey)
{

	switch (*_key)
	{
		case '1':
		{
		    *recommendedKey = '1';
		    *multiTap = IFX_FALSE;
		    break;
		}
		case '2':
		{
		    *recommendedKey = '2';
		    *multiTap = IFX_FALSE;
		    break;
		}
		case '3':
		{
		    *recommendedKey = '3';
		    *multiTap = IFX_FALSE;
		    break;
		}
		case '4':
		{
		    *recommendedKey = '4';
		    *multiTap = IFX_FALSE;
		    break;
		}
		case '5':
		{
		    *recommendedKey = '5';
		    *multiTap = IFX_FALSE;
		    break;
		}
		case '6':
		{
		    *recommendedKey = '6';
		    *multiTap = IFX_FALSE;
		    break;
		}
		case '7':
		{
		    *recommendedKey = '7';
		    *multiTap = IFX_FALSE;
		    break;
		}
		case '8':
		{
		    *recommendedKey = '8';
		    *multiTap = IFX_FALSE;
		    break;
		}
		case '9':
		{
		    *recommendedKey = '9';
		    *multiTap = IFX_FALSE;
		    break;
		}
		case '0':
		{
		    *recommendedKey = '0';
		    *multiTap = IFX_FALSE;
		    break;
		}
		case IFX_KEY_CODE_ASTERISK:
        {
			switch (*currentKey)
			{
				case '*':
				{
					*recommendedKey = '+';
					break;
				}
				case '+':
				{
					*recommendedKey = 'p';
					break;
				}
				case 'p':
				{
					*recommendedKey = 'w';
					break;
				}
				case 'w':
				{
					*recommendedKey = '*';
					break;
				}
				default:
				{
					*recommendedKey = '*';
					*multiTap = IFX_FALSE;
					break;
				}
            }
            break;
        }
        case IFX_KEY_CODE_HASH:
        {
		    *recommendedKey = '#';
		    *multiTap = IFX_FALSE;
		    break;
        }
        default:
		{
			*unknownKey	= IFX_TRUE;
			*multiTap	= IFX_FALSE;
			break;
		}
	}
}

/*-------------------------------------------------------------------------*//**

*/
static IFX_INT32 SimpleEdit_MultiTapKeyConvertor(	
											SIMPLE_EDIT_SESSION*	pSession,
											SIMPLE_EDIT*			pElement,
											IFX_INT32				key)
{

	IFX_INT32 inputMode			= pElement->inputMode;
	IFX_INT32 _key				= key;
	IFX_INT32 multiTap			= IFX_FALSE;
	IFX_INT32 currentKey		= _key;
	IFX_INT32 recommendedKey	= _key;
	IFX_INT32 unknownKey		= IFX_FALSE;
	IFX_UINT32 currentTime;
	static IFX_UINT32 oldTime = 0;
	
	SimpleEdit_Get_Current_Time(&currentTime);

	/* We can't check the buffer full status here yet as we
	 * don't know if it is a multi tap or not */

	/* The first time, we need to initialize oldTime to currentTime */
	if (oldTime == 0)
		oldTime = currentTime;

	/*	If there was a key pressed before this key then the we have to
		differentiate between new keypress and Key double press */
	if (pElement->caretPosition > 0)
	{
		multiTap	= IFX_TRUE;
		currentKey	= pElement->pBuffer [pElement->caretPosition -1];
		recommendedKey	= pElement->pBuffer [pElement->caretPosition -1];
	}

    /* If a control key was pressed last then we should not consider it a double press */
	if (pSession->lastKey == CONTROL_KEY)
	{
		currentKey = '\0';
		multiTap	= IFX_FALSE;
	}
	else if (oldTime + KEY_EXPIRE_TIME < currentTime)
	{//timed out, so cannot be a multitap
		currentKey = '\0';
		multiTap	= IFX_FALSE;
	}

    /* If we are not multi tap, then check the buffer status */
    if (multiTap == IFX_FALSE)
    {
		/* If the buffer is full then don't process the key */
		if (lc_wcslen (pElement->pBuffer) >= pElement->maxLength)
		{
			return '\0';
		}
    }

	/* Update the time of the last processed key */
	oldTime = currentTime;

	currentKey	= SimpleEdit_toLower(currentKey);

	if ((inputMode & SIMPLE_EDIT_INPUT_AUTOCAPWORD  ||
		inputMode & SIMPLE_EDIT_INPUT_AUTOCAPSENTENCE ||
		inputMode & SIMPLE_EDIT_INPUT_LOWERCASE ||
		inputMode & SIMPLE_EDIT_INPUT_UPPERCASE))
	{
		SimpleEdit_MultiTapTextKeyConvertor(&_key, &multiTap, &currentKey,
											&recommendedKey, &unknownKey);
	}
	else if (inputMode & SIMPLE_EDIT_INPUT_NUMERIC)
	{
		if (!( pElement->style & SIMPLE_EDIT_STYLE_NUMERIC &&
		(_key == IFX_KEY_CODE_ASTERISK || _key == IFX_KEY_CODE_HASH)))
		{
			SimpleEdit_MultiTapPhoneKeyConvertor(	&_key, &multiTap, &currentKey,
													&recommendedKey, &unknownKey);
		}
		else
		{
			unknownKey = IFX_TRUE;
		}
	}

	/* Key is a multi tap */
	if (multiTap == IFX_TRUE)
	{
		IFX_INT32 consumed;
		SimpleEdit_DeleteKey(pElement, BACKSPACE_KEY, &consumed);
		_key = recommendedKey;
	}
	else if (unknownKey == IFX_TRUE)
	{
		/* Don't update the unknown key. this might be a control key */
		_key = 0;
	}
	else
	{
		/* New character to append in buffer*/
		_key = recommendedKey;
	}

	return _key;
}

/*-------------------------------------------------------------------------*//**

*/
static IFX_RETURN_STATUS	SimpleEdit_NavigationKey(	SIMPLE_EDIT*	pElement,
														IFX_INT32		key,
														IFX_INT32*		pConsumed)
{
	/* Left to right */
	IFX_INT32 step = 1;
	IFX_INT32 pos = pElement->caretPosition;
	IFX_INT32 len = lc_wcslen(pElement->pBuffer);

	/* The default value of step is 1 */
	if (key == IFX_KEY_CODE_LEFT)
	{
		step = -1;
	}

	/* Take a step forward or backward */
	pos += step;

	/* Cannot go beyond text boundaries */
	if (pos < 0)
	{
		pos = 0;
	}
	else if (pos > len)
	{
		pos = len;
	}

	*pConsumed = IFX_TRUE;
	pElement->caretPosition = pos;

	return IFX_SUCCESS;
}

/*-------------------------------------------------------------------------*//**

*/
static IFX_RETURN_STATUS	SimpleEdit_AppendKey(	SIMPLE_EDIT*	pElement,
													IFX_INT32		key,
													IFX_INT32*		pConsumed)
{
	/* Key is a text key */
	if(key >= 32 && key <= 126)
	{
		IFX_INT32 len = lc_wcslen(pElement->pBuffer);

		/* If buffer has some capacity */
		if (key != 0 && len < pElement->maxLength && pElement->caretPosition >= 0)
		{
			IFX_INT32 rem = len - pElement->caretPosition;
			IFX_WCHAR *src = &pElement->pBuffer[len];
			IFX_WCHAR *des = &pElement->pBuffer[len + 1];

			/* Shifting the string one character forward */
			while (rem-- >= 0)
			{
				*des-- = *src--;
			}

			pElement->pBuffer[pElement->caretPosition] = key;
			pElement->caretPosition ++;

			if ( SimpleEdit_CheckSpecialChar( key ) == IFX_TRUE )
			{
				pElement->specialChars ++;
			}
		}

		*pConsumed = IFX_TRUE;

		return IFX_SUCCESS;
	}
	else
	{
		*pConsumed = IFX_FALSE;

		return IFX_ERROR;
	}
}

/*-------------------------------------------------------------------------*//**

*/
static IFX_RETURN_STATUS	SimpleEdit_DeleteKey(	SIMPLE_EDIT*	pElement,
													IFX_INT32		key,
													IFX_INT32*		pConsumed)
{
	IFX_INT32 len = lc_wcslen(pElement->pBuffer);

	/* If buffer has some character and caret is */
	if (len > 0 && pElement->caretPosition > 0)
	{
        if ( SimpleEdit_CheckSpecialChar (pElement->pBuffer[len - 1]) == IFX_TRUE )
        {
            pElement->specialChars--;
        }
		/* Deleting last character */
		if (pElement->caretPosition >= len)
		{
			pElement->pBuffer[len - 1] = '\0';
		}
		else
		{
			/* Deleting other than last character */
			lc_wcscpy (&pElement->pBuffer[pElement->caretPosition - 1],
					&pElement->pBuffer[pElement->caretPosition]);
		}

		pElement->caretPosition--;
	}

	*pConsumed = IFX_TRUE;

	return IFX_SUCCESS;
}

/*-------------------------------------------------------------------------*//**

*/
static IFX_RETURN_STATUS	SimpleEdit_ControlKey(	SIMPLE_EDIT_SESSION*	pSession,
													SIMPLE_EDIT*			pElement,
													IFX_INT32				key,
													IFX_INT32*				pConsumed,
													IFX_INT32*				pCallback)
{
	IFX_RETURN_STATUS status = IFX_SUCCESS;
	IFX_INT32 old_key = pSession->lastKey;
	*pConsumed = IFX_TRUE;
	*pCallback = IFX_TRUE;

	pSession->lastKey = CONTROL_KEY;

	switch(key)
	{
		case BACKSPACE_KEY:
		{
			status = SimpleEdit_DeleteKey(pElement, key, pConsumed);
			break;
		}

		case INPUT_MODE_KEY_CHANGE:
		{
			if (pElement->style & SIMPLE_EDIT_STYLE_TEXT)
			{
				status = SimpleEdit_CycleInputMode(pSession, pElement);
				*pCallback = IFX_FALSE;
			}
			else
			{
			    status				= IFX_ERROR;
			    *pConsumed			= IFX_FALSE;
			    pSession->lastKey   = old_key;
			}
			break;
		}

		case IFX_KEY_CODE_LEFT:
		case IFX_KEY_CODE_RIGHT:
		{
			status = SimpleEdit_NavigationKey(pElement, key, pConsumed);
			break;
		}

		// For these control keys, return error 
		// but leave the lastKey as a control key.
		case IFX_KEY_CODE_UP:
		case IFX_KEY_CODE_DOWN:
		{
			status				= IFX_ERROR;
			*pConsumed			= IFX_FALSE;
			break;
		}

		default:
		{
			status				= IFX_ERROR;
			*pConsumed			= IFX_FALSE;
			pSession->lastKey   = old_key;
			break;
		}
	}

	return status;
}

/*-------------------------------------------------------------------------*//**

*/
IFX_RETURN_STATUS	SimpleEdit_ProcessElementMultiTapKeyEvent(	SIMPLE_EDIT_SESSION*	pSession,
																SIMPLE_EDIT*			pElement,
																IFX_INT32				key,
																IFX_INT32*				pConsumed,
																IFX_INT32*				pCallback)
{
	IFX_RETURN_STATUS status = IFX_ERROR;
	*pConsumed = IFX_FALSE;

	if (pElement->caretPosition < 0)
		return status;

	status = SimpleEdit_ControlKey(pSession, pElement, key, pConsumed, pCallback);

	if (status == IFX_ERROR)
	{
		key = SimpleEdit_MultiTapKeyConvertor(pSession, pElement, key);

		if (key != '\0')
		{
			key = SimpleEdit_TextKeyCaseHandler(pElement, key);
			status = SimpleEdit_AppendKey(pElement, key, pConsumed);
			pSession->lastKey = TEXT_KEY;
		    status = IFX_SUCCESS;
		    *pConsumed = IFX_TRUE;

			*pCallback = IFX_TRUE;
		}
	}

	return status;
}
