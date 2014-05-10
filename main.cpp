﻿#define DISASSEM 01
#define ASSEM    0
#define TEST     0
#define TEST2    0

#include <iostream>
#include <sstream>
#include <QVector>
#include <QString>
#include <QFile>
#include <QRegExp>
#include <QDebug>
#include <QTextCodec>
#include <QByteArray>
#include "assembler.h"
#include "lookuptable.h"

#if TEST2


int main(void)
{
    quint32 instruction;
    instruction=0xf0;
    qDebug()<<((instruction>>4)&0x1);

    return 0;
}
#endif
#if DISASSEM
int main(void)
{
QString filename("/Users/ying/assemble");
    QFile fileObj(filename+".objj");
    if(!fileObj.open(QIODevice::ReadOnly )){
        qDebug()<<"Error in opening object file";
        return -1;
    }
    //QTextStream streamObj(&fileObj);
    QDataStream streamObj(&fileObj);
    streamObj.setVersion(QDataStream::Qt_5_2);

    QFile fileDisassem(filename+".disasm");
    if(!fileDisassem.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug()<<"Error in writing to disassemble file";
        return -1;
    }
    QTextStream streamDisassem(&fileDisassem);


    //unsigned int instruction;
    quint32 instruction;
    MatchTable matchTable;
    int matchId;
    while(!streamObj.atEnd())
    {
        streamObj>>instruction;

        matchId=matchTable.DisassemMatchRegister(instruction>>26, instruction & 0x3f);
        switch (matchId) {
        case 0: case 3: case 4: case 16: case 17: case 19: case 22: case 29: case 30:
            //add,addu,and,nor,or,slt,sltu,sub,subu
            streamDisassem<<coreInstructionSet[matchId].mnemonic<<" $"
                            <<((instruction>>11)&0x1f)<<", $"
                           <<((instruction>>21)&0x1f)<<", $"
                             <<((instruction>>16)&0x1f)<<endl;
            break;

        case 1: case 2: case 5: case 18: case 20: case 21:
            //addi,addiu,andi,ori,slti,sltiu
            streamDisassem<<coreInstructionSet[matchId].mnemonic<<" $"
                            <<((instruction>>16)&0x1f)<<", $"
                           <<((instruction>>21)&0x1f)<<", "
                             <<(instruction&0xffff)<<endl;
            break;

        case 6: case 7:
            //beq,bne
            streamDisassem<<coreInstructionSet[matchId].mnemonic<<" $"
                            <<((instruction>>21)&0x1f)<<", $"
                           <<((instruction>>16)&0x1f)<<", "
                             <<hex<<(instruction&0xffff)<<reset<<endl;
            break;

        case 8: case 9:
            //j,jal
            streamDisassem<<coreInstructionSet[matchId].mnemonic<<" "
                          <<hex<<(instruction&0x3ffffff)<<reset<<endl;
            break;

        case 10:
            //jr
            streamDisassem<<coreInstructionSet[matchId].mnemonic<<" $"
                          <<((instruction>>21)&0x1f)<<endl;
            break;

        case 11: case 12: case 13: case 15: case 25: case 26: case 27: case 28:
            //lbu,lhu,ll,lw,sb,sc,sh,sw
            streamDisassem<<coreInstructionSet[matchId].mnemonic<<" $"
                            <<((instruction>>16)&0x1f)<<", "
                           <<(instruction&0xffff)<<"($"
                             <<((instruction>>21)&0x1f)<<")"<<endl;
            break;

        case 14:
            //lui
            streamDisassem<<coreInstructionSet[matchId].mnemonic<<" $"
                            <<((instruction>>16)&0x1f)<<", "
                           <<(instruction&0xffff)<<endl;
            break;

        case 23: case 24:
            //sll,srl
            streamDisassem<<coreInstructionSet[matchId].mnemonic<<" $"
                            <<((instruction>>11)&0x1f)<<", $"
                           <<((instruction>>16)&0x1f)<<", "
                             <<((instruction>>6)&0x1f)<<endl;
            break;

        default:
            qDebug()<<"Unknown instruction"<<hex<<instruction<<reset;
            break;
        }

    }

    fileObj.close();
    fileDisassem.close();

    return 0;
}

#endif

#if ASSEM
int Assem(QString filename)
{
    QFile fileXml(filename+".xml");
    if(!fileXml.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"Error in opening temporary file";
        return -1;
    }
    QTextStream streamXml(&fileXml);

    QFile fileObj(filename+".obj");
    if(!fileObj.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug()<<"Error in writing to object file";
        return -1;
    }
    QTextStream streamObj(&fileObj);

    QString line;
    unsigned int lineNumber;
    unsigned int instructionAddress=0;
    unsigned int instruction=0;
    MatchTable matchTable;
    int matchId;
    unsigned short int rs, rt, rd, shamt;
    unsigned int address;
    int immediate;
    LookUpTable labelLookUpTable;
    labelLookUpTable.Load(filename);

    while(!streamXml.atEnd()){
        streamXml>>line;

        if(line.compare("<blankline/>")==0)
            continue;
        else if(line.compare("<linenumber>")==0)
        {
            streamXml>>line;
            lineNumber=line.toInt();
        }
        else if(line.compare("<instruction>")==0)
        {
            streamXml>>line;
            matchId=matchTable.MatchInstruction(line);

            if(matchId<0)
            {
                qDebug()<<line<<": Unknown/unsupported instruction name";
                continue;
            }
            instruction=coreInstructionSet[matchId].opcode<<26
                      | coreInstructionSet[matchId].funct;

            streamXml.readLine();
            switch (matchId) {
            case 0: case 3: case 4: case 16: case 17: case 19: case 22: case 29: case 30:
                //add,addu,and,nor,or,slt,sltu,sub,subu
                rd=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>",labelLookUpTable);
                rs=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>",labelLookUpTable);
                rt=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>",labelLookUpTable);
                instruction=instruction | rs<<21 | rt<<16 | rd<<11;
                break;

            case 1: case 2: case 5: case 18: case 20: case 21:
                //addi,addiu,andi,ori,slti,sltiu
                rt=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>",labelLookUpTable);
                rs=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>",labelLookUpTable);
                immediate=(unsigned short int)matchTable.instructionEncode(streamXml,"<parameter>",labelLookUpTable);
                instruction=instruction | rs<<21 | rt<<16 | immediate;
                break;

            case 6: case 7:
                //beq,bne
                rs=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>",labelLookUpTable);
                rt=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>",labelLookUpTable);
                address=(unsigned short int)matchTable.instructionEncode(streamXml,"<reference>",labelLookUpTable);
                instruction=instruction | rs<<21 | rt<<16 | address;
                break;

            case 8: case 9:
                //j,jal
                address=(unsigned short int)matchTable.instructionEncode(streamXml,"<reference>",labelLookUpTable);
                instruction=instruction | address;
                break;

            case 10:
                //jr
                rs=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>",labelLookUpTable);
                instruction=instruction | rs<<21;
                break;

            case 11: case 12: case 13: case 15: case 25: case 26: case 27: case 28:
                //lbu,lhu,ll,lw,sb,sc,sh,sw
                rt=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>",labelLookUpTable);
                immediate=(unsigned short int)matchTable.instructionEncode(streamXml,"<parameter>",labelLookUpTable);
                rs=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>",labelLookUpTable);
                instruction=instruction | rs<<21 | rt<<16 | immediate;
                break;

            case 14:
                //lui
                rt=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>",labelLookUpTable);
                immediate=(unsigned short int)matchTable.instructionEncode(streamXml,"<parameter>",labelLookUpTable);
                instruction=instruction | rt<<16 | immediate;
                break;

            case 23: case 24:
                //sll,srl
                rd=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>",labelLookUpTable);
                rt=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>",labelLookUpTable);
                shamt=(unsigned short int)matchTable.instructionEncode(streamXml,"<parameter>",labelLookUpTable);
                instruction=instruction | rs<<21 | rt<<16 | rd<<11 | shamt<<6;
                break;

            default:
                break;
            }

            instructionAddress+=4;
            streamObj<<qSetFieldWidth(8)<<qSetPadChar('0')<<hex<<instruction;
            streamObj<<reset;
            streamObj<<endl;
        }

        line=streamXml.readLine();


    }

    fileXml.close();
    fileObj.close();

    return 0;
}


int Parser(QString filename)
{
    //QTextCodec *codecName=QTextCodec::codecForUtfText("UTF-8");
    //QTextCodec::setCodecForLocale(codecName);
    //QString filename;
    //filename="assemble";
    QFile fileAsm(filename);
    if(!fileAsm.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"Error in opening ASM file";
        return -1;
    }
    QTextStream streamAsm(&fileAsm);

    QFile fileXml(filename+".xml");
    if(!fileXml.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug()<<"Error in writing to output file";
        return -1;
    }
    QTextStream streamXml(&fileXml);

    QRegExp instructionPattern("\\s*([a-zA-Z]+)\\s*");
    instructionPattern.setCaseSensitivity(Qt::CaseInsensitive);
    QRegExp registerPattern("[,\\s\\(]*(\\$\\w+)\\s*[,\\)]*\\s*");
    registerPattern.setCaseSensitivity(Qt::CaseInsensitive);
    QRegExp parameterPattern("[,\\s]*([\\+-\\w]+)[,\\(]*\\s*");
    parameterPattern.setCaseSensitivity(Qt::CaseSensitive);
    QRegExp labelPattern("\\s*(\\w+\\s*):\\s*");
    labelPattern.setCaseSensitivity(Qt::CaseSensitive);
    QRegExp blanklinePattern("^\\s*$");

    QString line;
    int lineNumber=0;
    unsigned int address=0;
    LookUpTable labelLookUpTable;
    while (!streamAsm.atEnd()) {
        streamXml<<"<linenumber> "<<++lineNumber<<" </linenumber>"<<endl;
        line=streamAsm.readLine();
        if(blanklinePattern.indexIn(line)>=0) {
            streamXml<<"<blankline/>"<<endl;
            continue;
        }

        int matchPosition=0;
        int position=0;
        short labelFlag=0;
        short instructionFlag=0;
        while(position<line.length())
        {
            if(line[position]=='#' || (line[position]=='/'&&line[position+1]=='/'))
                break;

            if(labelFlag==0)
            {
                matchPosition=labelPattern.indexIn(line,position);
                if(matchPosition>=0)
                {
                    streamXml<<"<label> "<<labelPattern.capturedTexts()[1]
                            <<" </label>"<<endl;
                    labelLookUpTable.Push(labelPattern.capturedTexts()[1],address);
                    position=matchPosition+labelPattern.matchedLength();
                    labelFlag=1;
                    continue;
                }
            }


            if(instructionFlag==0)
            {
                matchPosition=instructionPattern.indexIn(line,position);
                if(matchPosition>=0)
                {
                    streamXml<<"\t<instruction> "<<instructionPattern.capturedTexts()[1]
                            <<" </instruction>"<<endl;
                    position=matchPosition+instructionPattern.matchedLength();

                    instructionFlag=1;
                    address+=4;  //address counting
                    continue;
                }
            }

            //consider to use pattern.pos() to judge
            if(line[position]=='$')
            {
                matchPosition=registerPattern.indexIn(line,position);
                if(matchPosition>=0)
                {
                    streamXml<<"\t\t<register> "<<registerPattern.capturedTexts()[1]
                            <<" </register>"<<endl;
                    position=matchPosition+registerPattern.matchedLength();
                    continue;
                }
            }

            matchPosition=parameterPattern.indexIn(line,position);
            if(matchPosition>=0)
            {
                if((line[position]>='a'&&line[position]<='z')
                        ||(line[position]>='A'&&line[position]<='Z'))
                    streamXml<<"\t\t<reference> "<<parameterPattern.capturedTexts()[1]
                            <<" </reference>"<<endl;
                else
                    streamXml<<"\t\t<parameter> "<<parameterPattern.capturedTexts()[1]
                            <<" </parameter>"<<endl;
                position=matchPosition+parameterPattern.matchedLength();
                continue;
            }

        }

        if(position<line.length())
        {
            streamXml<<"<comment> "<<line.mid(position,line.length())
                    <<" </comment>"<<endl;
        }
    }

    labelLookUpTable.Save(filename);

    fileAsm.close();
    fileXml.close();

    return 0;
}


int main(void)
{
    QString filename("/Users/ying/assemble");

    Parser(filename);
    Assem(filename);

    return 0;
}

#endif

#if TEST
void foo(QTextStream &stream)
{
    QString line;
    line=stream.readLine();
    qDebug()<<line;

    return;
}

int main(void)
{
    QFile fileAsm("/Users/ying/assemble");
    if(!fileAsm.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"Error in opening ASM file";
    }
    QTextStream streamAsm(&fileAsm);


    QRegExp instructionPattern;
    instructionPattern.setPattern("^\\$([a-zA-Z]+\\d*)$");
    instructionPattern.setCaseSensitivity(Qt::CaseInsensitive);
    QRegExp labelPattern("\\s*(\\$[a-zA-Z]*\\d*)\\s*");
    labelPattern.setCaseSensitivity(Qt::CaseSensitive);

    QString line;
    line=streamAsm.readLine();
    qDebug()<<line;
    foo(streamAsm);
    line=streamAsm.readLine();
    qDebug()<<line;
    instructionPattern.indexIn(line);
    std::cout<<instructionPattern.capturedTexts()[1].toStdString();

    fileAsm.close();
//        instructionPattern.indexIn(line);
//        qDebug()<<instructionPattern.capturedTexts();

}

#endif


