#pragma once

#include "fb/lang/Types.h"
#include "fb/string/HeapString.h"

/* Conversions from lower_case to camelCase and back 
 *
 * Note: This only works for ASCII characters (a to z). No attempt to handle international characters is made
 */

FB_PACKAGE2(string, detail)

/**
 * Converts string from CamelCase to lower_case
 */
static void convertCamelCaseToLowerCase(HeapString &strInOut)
{
	if (strInOut.isEmpty())
		return;

	bool lastWasUpperCase = strInOut.isUpper(0);
	bool lastWasLowerCase = strInOut.isUpper(0);

	// convert first char to plain lower case
	if (strInOut.isUpper(0))
	{
		strInOut.toLower(0);
	}

	// prefix other upper case characters with underscore
	for (unsigned int i = 1; i < strInOut.getLength(); i++)
	{
		if (isUpper(strInOut[i]))
		{
			strInOut[i] = toLower(strInOut[i]);
			// this is to convert StuffLikeGUICrap properly into stuff_like_gui_crap
			bool nextIsLowerCase = i+1 < strInOut.getLength() && isLower(strInOut[i+1]);
			if (lastWasLowerCase || (lastWasUpperCase && nextIsLowerCase))
			{
				strInOut.insert(i, "_", 1);
				i++;
			}
			lastWasLowerCase = false;
			lastWasUpperCase = true;
		}
		else
		{
			lastWasUpperCase = isUpper(strInOut[i]);
			lastWasLowerCase = isLower(strInOut[i]);
		}
	}
}

/**
* Converts string from lower_case to CamelCase
*/
static void convertLowerCaseToCamelCase(HeapString &strInOut)
{
	if (strInOut.isEmpty())
		return;

	if (isLower(strInOut[0]))
	{
		strInOut[0] = toUpper(strInOut[0]);
	}

	for (SizeType i = strInOut.getLength() - 1U; i-- > 1;)
	{
		if (strInOut[i] == '_')
		{
			strInOut.erase(i, 1);
			if (isLower(strInOut[i]))
				strInOut[i] = toUpper(strInOut[i]);
		}
	}
}

/**
* Converts string from lower_case to Spaced Capitalized
*/
static void convertUnderScoredToSpacedCapitalized(HeapString &stringInOut)
{
	bool capitalizeNext = true;
	for (SizeType i = 0, len = stringInOut.getLength(); i < len; ++i)
	{
		if (stringInOut[i] == '_')
		{
			stringInOut[i] = ' ';
			capitalizeNext = true;
		}
		else if (capitalizeNext)
		{
			stringInOut.toUpper(i);
			capitalizeNext = false;
		}
	}
}

FB_END_PACKAGE2()
