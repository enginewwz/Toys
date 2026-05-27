// not tested
// Hash list with index ranging from 0 to 19999 to store 15000 values

int Hash (unsigned char s)
{
    int c1c2 = (int)s / 1000000;
    int c3c4 = (int)s % 1000000;
    int c5 = c3c4 % 10000;
    c3c4 /= 10000;
    int c6c7c8 = c5 % 1000;
    c5 = c5 / 1000;
    if (c5 && (c6c7c8 < 20) || (!c5 && (c6c7c8 < 130)))
    {
        return (c1c2 - 96) * 5000 + c3c4 * 250 + c5 * 130 + c6c7c8;
    }
    else
    {
        return (c1c2 - 96) * 5000 + c5 * (1000 + 110) + c3c4 * 30 + (c6c7c8 - 130);
    }
}

void InsertHash (unsigned char s, int* HashTable)
{
    int p = Hash(s);
    int offset = p % 5000 - 3750;
    int base = p - offset;
    int chkoffset = offset;
    if (offset < 0)
        HashTable[p] = s;
    else if (HashTable[p] == 0)  // the place is null
        HashTable[p] = s;
    else
    {
        for (offset ++; HashTable[p + offset] != 0 && offset != chkoffset; offset = (offset + 1) % 1250)
        {
            if (HashTable[p + offset] == s)
                return;  // already exists
        }
        if (offset == chkoffset)
            printf("full");
        else
            HashTable[p + offset] = s;
    }
}