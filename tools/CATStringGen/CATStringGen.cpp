// Quick util to gen error/string table header from CATStringCore.xml
#include <stdio.h>
#include "CAT.h"

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("Usage: CATStringGen StringFile.xml\n");
		return 1;
	}

	CATStringTableCore strTable;
	CATResult res = strTable.GenHeaderForXML(argv[1]);
	if (CATFAILED(res))
	{
		printf("Error 0x%08x generating header.",res);
		return 3;
	}
	return 0;
}
