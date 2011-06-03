#include "CAT.h"
#include "CATStringTableCore.h"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("CATXML stringtable processor. Usage: CATXML stringtable.xml\n");
        return -1;
    }
    CATStringTableCore cst;
    CATResult result = cst.GenHeaderForXML(argv[1]);
    if (CATFAILED(result))
    {
        printf("Failed to load: %s\n",argv[1]);
        return -2;
    }
    printf("Header generated for %s\n",argv[1]);


    return 0;
}