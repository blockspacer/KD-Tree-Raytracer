#pragma once

FB_PACKAGE1(string)

/**
 * Replaces all markers ( {0}, {1}, {2}, {3}...) with the given parameters
 * Use {{ and }} for actual { and } respectively
 */
template<class Vector>
void replaceMarkers(HeapString &stringInOut, const Vector &stringParameters)
{
	for (SizeType i = 0; i + 1 < stringInOut.getLength(); ++i)
	{
		if (stringInOut[i] == '{')
		{
			if (stringInOut[i + 1] == '{')
			{
				stringInOut.erase(i, 1);
			}
			else
			{
				SizeType paramIndex = (SizeType)atoi(&stringInOut[i + 1]);
				if (paramIndex >= stringParameters.getSize())
					continue;

				SizeType count = 1;
				while (i + count + 1 < stringInOut.getLength() && stringInOut[i + count] != '}')
				{
					++count;
				}
				stringInOut.erase(i, count + 1);

				stringInOut.insert(i, stringParameters[paramIndex].getPointer(), stringParameters[paramIndex].getLength());
				i += stringParameters[paramIndex].getLength() - 1;
			}
		}
		else if (stringInOut[i] == '}')
		{
			if (stringInOut[i + 1] == '}')
			{
				stringInOut.erase(i, 1);
			}
		}
	}
}

FB_END_PACKAGE1()
