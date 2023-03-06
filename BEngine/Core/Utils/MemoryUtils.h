inline void BitshiftMemory ( char* const first, char* last )
{
    for ( char* p = first; p != last; p++ )
    {
        char value = *p;
        value = (value & 1) << 1;
        *p = value;
    }
}