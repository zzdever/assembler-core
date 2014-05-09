#include <QString>
#include <QRegExp>
#include <QDebug>
#include <iostream>
#include "assembler.h"

MatchTable::MatchTable(void)
{
    registerPatternName.setPattern("^\\$([a-zA-Z]+\\d*)$");
    registerPatternName.setCaseSensitivity(Qt::CaseInsensitive);
    registerPatternNumber.setPattern("^\\$(\\d+)$");
}

int MatchTable::MatchInstruction(QString instruct)
{
    for(int i=0;i<INSTRUCTIONSETSIZE;i++)
    {
        if(0==instruct.compare(coreInstructionSet[i].mnemonic)) return i;
    }

    return -1;
}

int MatchTable::MatchRegister(QString registerName)
{
    registerName=registerName.toLower();

    if(registerPatternNumber.indexIn(registerName)>=0)
        return registerPatternNumber.capturedTexts()[1].toInt();
    else if(registerPatternName.indexIn(registerName)>=0)
    {
        registerName=registerPatternName.capturedTexts()[1];
        for(int i=0;i<REGISTERSETSIZE;i++)
        {
            if(0==registerName.compare(registerSet[i].name)) return i;
        }
    }

    return -1;
}
