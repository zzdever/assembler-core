#define ASSEM 01
#define PARSE 0
#define TEST  0
#define TEST2 0

#include <iostream>
#include <QVector>
#include <QString>
#include <QFile>
#include <QRegExp>
#include <QDebug>
#include <QTextCodec>
#include "assembler.h"

#if ASSEM
void InstructionCompose(QTextStream &streamXml,QVector<QString*> tokenList)
{
    QString line;
    MatchTable matchTable;
    int matchId;

    for(int i=0;i<tokenList.size();i++)
    {
        if(tokenList[i]==NULL) break;
        streamXml>>line;

        if(line.compare("<register>")==0)
        {
            streamXml>>line;
            matchId=matchTable.MatchRegister(line);

            if(matchId<0)
                qDebug()<<"Unknown register number/name";
            else
                *(tokenList[i])=QString(registerSet[matchId].number);
qDebug()<<"register match:"<<registerSet[matchId].number;
        }
        else if(line.compare("<parameter>")==0)
        {
            streamXml>>line;
    qDebug()<<"parameter match:"<<line.toInt();
            *(tokenList[i])=line.toInt();
        }
        else if(line.compare("<reference>")==0)
        {
            //tokenList[i];
        }
        else
            qDebug()<<"Unknown operand type or wrong instruction";

        streamXml.readLine();
    }
    return;
}

unsigned short int instructionEncode(QTextStream &streamXml, QString type)
{
    ;
}

int main(void)
{
    QFile fileXml("/Users/ying/assemble.xml");
    if(!fileXml.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"Error in opening temporary file";
    }
    QTextStream streamXml(&fileXml);

    QFile fileObj("/Users/ying/assemble.obj");
    if(!fileObj.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug()<<"Error in writing to objet file";
    }
    QTextStream streamObj(&fileObj);

    QString line;
    unsigned int lineNumber;
    unsigned int instructionAddress=0;
    unsigned int instruction=0;
    MatchTable matchTable;
    int matchId;
    QString rs, rt ,rd, shamt, immediate, address;
    QVector<QString*>tokenList(6,NULL);
    while(!streamXml.atEnd()){
        streamXml>>line;
qDebug()<<line;

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

            if(matchId<=0) qDebug()<<"Unknown/unsupported instruction name";
            instruction=coreInstructionSet[matchId].opcode<<26
                      | coreInstructionSet[matchId].funct;

            streamXml.readLine();
            tokenList.fill(NULL);
            switch (matchId) {
            case 0: case 3: case 4: case 16: case 17: case 19: case 22: case 29: case 30:
                //add,addu,and,nor,or,slt,sltu,sub,subu
                tokenList[0]=&rd;
                tokenList[1]=&rs;
                tokenList[2]=&rt;
                InstructionCompose(streamXml,tokenList);
                instruction=instruction
                        | rs.toInt()<<21 | rt.toInt()<<16 | rd.toInt()<<11;
                break;

            case 1: case 2: case 5: case 18: case 20: case 21:
                //addi,addiu,andi,ori,slti,sltiu
                tokenList[0]=&rt;
                tokenList[1]=&rs;
                tokenList[2]=&immediate;
                InstructionCompose(streamXml,tokenList);
                instruction=instruction
                        | rs.toInt()<<21 | rt.toInt()<<16 | immediate.toInt();
                break;

            case 6: case 7: //beq,bne
                tokenList[0]=&rs;
                tokenList[1]=&rt;
                tokenList[2]=&address;
                InstructionCompose(streamXml,tokenList);
                instruction=instruction
                        | rs.toInt()<<21 | rt.toInt()<<16 | address.toInt();
                break;

            case 8: case 9: //j,jal
                tokenList[0]=&address;
                InstructionCompose(streamXml,tokenList);
                instruction=instruction | address.toInt()<<26;
                break;

            case 10: //jr
                tokenList[0]=&rs;
                InstructionCompose(streamXml,tokenList);
                instruction=instruction | rs.toInt()<<21;
                break;

            case 11: case 12: case 13: case 15: case 25: case 26: case 27: case 28:
                //lbu,lhu,ll,lw,sb,sc,sh,sw
                tokenList[0]=&rt;
                tokenList[1]=&immediate;
                tokenList[2]=&rs;e
                InstructionCompose(streamXml,tokenList);
    qDebug()<<"rt:"<<rt.toInt()<<"imme: "<<immediate.toInt()<<"rs:"<<rs.toInt();
                instruction=instruction
                        | rs.toInt()<<21 | rt.toInt()<<16 | immediate.toInt();
                break;

            case 14: //lui
                tokenList[0]=&rt;
                tokenList[1]=&immediate;
                InstructionCompose(streamXml,tokenList);
                instruction=instruction
                        | rt.toInt()<<16 | immediate.toInt();
                break;

            case 23: case 24: //sll,srl
                tokenList[0]=&rd;
                tokenList[1]=&rt;
                tokenList[2]=&shamt;
                InstructionCompose(streamXml,tokenList);
                instruction=instruction
                        | rs.toInt()<<21 | rt.toInt()<<16
                                      | rd.toInt()<<11 | shamt.toInt()<<6;
                break;

            default:
                break;
            }

            instructionAddress+=4;
            qDebug()<<instruction;
            streamObj<<qSetFieldWidth(8)<<hex<<instruction<<endl;
        }
        else if(line.compare("<label>"))
        {
            ;
        }

        line=streamXml.readLine();


    }

    fileXml.close();
    fileObj.close();

    return 0;
}
#endif
#if PARSE
int main(void)
{
    //QTextCodec *codecName=QTextCodec::codecForUtfText("UTF-8");
    //QTextCodec::setCodecForLocale(codecName);
    //QString filename;
    //filename="assemble";
    QFile fileAsm("/Users/ying/assemble");
    if(!fileAsm.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"Error in opening ASM file";
    }
    QTextStream streamAsm(&fileAsm);

    QFile fileXml("/Users/ying/assemble.xml");
    if(!fileXml.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug()<<"Error in writing to output file";
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
        {qDebug()<<"position:  "<<position;
            if(line[position]=='#' || (line[position]=='/'&&line[position+1]=='/'))
                break;

            if(labelFlag==0)
            {
                matchPosition=labelPattern.indexIn(line,position);
                if(matchPosition>=0)
                {
                    qDebug()<<labelPattern.capturedTexts();
                    streamXml<<"<label> "<<labelPattern.capturedTexts()[1]
                            <<" </label>"<<endl;
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
                    qDebug()<<instructionPattern.capturedTexts();
                    streamXml<<"\t<instruction> "<<instructionPattern.capturedTexts()[1]
                            <<" </instruction>"<<endl;
                    position=matchPosition+instructionPattern.matchedLength();

                    instructionFlag=1;
                    continue;
                }
            }

            //consider to use pattern.pos() to judge
            if(line[position]=='$')
            {
                matchPosition=registerPattern.indexIn(line,position);
                if(matchPosition>=0)
                {qDebug()<<registerPattern.capturedTexts();
                    streamXml<<"\t\t<register> "<<registerPattern.capturedTexts()[1]
                            <<" </register>"<<endl;
                    position=matchPosition+registerPattern.matchedLength();
                    continue;
                }
            }

            matchPosition=parameterPattern.indexIn(line,position);
            if(matchPosition>=0)
            {qDebug()<<parameterPattern.capturedTexts();
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
            //line.;
            //streamXml.codec("utf8");      &line.toStdString()[position]
            streamXml<<"<comment> "<<line.mid(position,line.length())
                    <<" </comment>"<<endl;
        }
    }

    fileAsm.close();
    fileXml.close();

//    QRegExp instruction("\\s*(\\w+)\\s+([\\w$]+)\\s*,\\s*([\\w+$]+)\\s*(.*)");

    return 0;
}

#endif

#if TEST
int main(void)
{
    QFile fileAsm("/home/dever/ch");
    if(!fileAsm.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"Error in opening ASM file";
    }
    QTextStream streamAsm(&fileAsm);


    QRegExp instructionPattern;
    instructionPattern.setPattern("^\\$([a-zA-Z]+\\d*)$");
    instructionPattern.setCaseSensitivity(Qt::CaseInsensitive);
    QRegExp labelPattern("\\s*(\\$[a-zA-Z]*\\d*)\\s*");
    labelPattern.setCaseSensitivity(Qt::CaseSensitive);



    QString line("$T3");
    line=line.toLower();
    qDebug()<<line;
    instructionPattern.indexIn(line);
    std::cout<<instructionPattern.capturedTexts()[1].toStdString();

    fileAsm.close();
//        instructionPattern.indexIn(line);
//        qDebug()<<instructionPattern.capturedTexts();

}

#endif

#if TEST2
void foo(int a)
{
    qDebug()<<a;
    return;
}

void foo(char a)
{
    qDebug()<<a;
    return;
}

int main(void)
{
    int a=9;
    char c='d';

    foo(a);
    foo(c);

    return 0;
}
#endif
