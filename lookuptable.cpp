#include "lookuptable.h"

LookUpTable::LookUpTable()
{
    head = new labelTable[MAXLABELAMOUNT];
    labelAmount=0;
}

unsigned int LookUpTable::lookUp(QString name)
{
    for(int i=0; i<labelAmount;i++)
    {
        if(name == head[i].name)
            return head[i].address;
    }
    return -1;
}

void LookUpTable::Push(QString name, int addr)
{
    if(labelAmount>=MAXLABELAMOUNT)
    {
        qDebug()<<"Label lookup table is full!";
        return;
    }

    head[labelAmount].name=name;
    head[labelAmount].address=addr;
    labelAmount++;

    return;
}

void LookUpTable::PrintAll(void)
{
    for(int i=0;i<labelAmount;i++)
    {
        qDebug()<<"label: "<<head[i].name<<"  address: "<<head[i].address;
    }

    return;
}
