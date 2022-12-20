extern void puti(int i);

bool is_munch(int a)
{
    if(a==a*10%10)
    {
        return true;
    }
    
    return false;
}

void test()
{
    for(int i=1; i<1; ++i)
    {
        if(is_munch(i))
        {
            puti(i);
        }
    }
}