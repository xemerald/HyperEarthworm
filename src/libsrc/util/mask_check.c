#include <stdio.h>

char ip_in_same_netmask(char * ip1, char * ip2, char *netmask)
{
    unsigned int ipA, ipB, ipC, ipD;
    unsigned int testA, testB, testC, testD;
    unsigned int maskA, maskB, maskC, maskD;
    sscanf(ip1, "%u.%u.%u.%u", &ipA, &ipB, &ipC, &ipD);
    sscanf(ip2, "%u.%u.%u.%u", &testA, &testB, &testC, &testD);
    sscanf(netmask, "%u.%u.%u.%u", &maskA, &maskB, &maskC, &maskD);
    if (((ipA & maskA) == (testA & maskA)) && ((ipB & maskB) == (testB & maskB)) && ((ipC & maskC) == (testC & maskC)) && ((ipD & maskD) == (testD & maskD)))
    {
        return 1; /*true*/
    }
    return 0; /*false*/
}
