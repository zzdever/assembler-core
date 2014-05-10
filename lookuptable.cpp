#include <QFile>
#include "lookuptable.h"

LookUpTable::LookUpTable()
{
    head = new labelTable[MAXLABELAMOUNT];
    labelAmount=0;
}

void LookUpTable::Load(QString filename)
{
    QFile fileLabelLookUpTable(filename+".table");
    if(!fileLabelLookUpTable.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"Error in reading label lookup table file";
    }
    QTextStream streamLabelLookUpTable(&fileLabelLookUpTable);

    int i=0;
    while(!streamLabelLookUpTable.atEnd())
    {
        streamLabelLookUpTable>>head[i].name;
        streamLabelLookUpTable>>head[i].address;
        i++;
        labelAmount++;
    }

    fileLabelLookUpTable.close();

    return;
}

unsigned int LookUpTable::LookUp(QString name)
{
    for(int i=0; i<labelAmount;i++)
    {
        if(name == head[i].name)
            return head[i].address;
    }
    return -1;  //0xffffffff
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

void LookUpTable::Save(QString filename)
{
    QFile fileLabelLookUpTable(filename+".table");
    if(!fileLabelLookUpTable.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug()<<"Error in writing to label lookup table file";
    }
    QTextStream streamLabelLookUpTable(&fileLabelLookUpTable);

    for(int i=0;i<labelAmount;i++)
    {
        streamLabelLookUpTable<<head[i].name<<" "<<head[i].address<<endl;
    }

    fileLabelLookUpTable.close();

    return;
}
