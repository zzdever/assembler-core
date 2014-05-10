#define ASSEM 0
#define PARSE 01
#define TEST  0
#define TEST2 0

#include <iostream>
#include <sstream>
#include <QVector>
#include <QString>
#include <QFile>
#include <QRegExp>
#include <QDebug>
#include <QTextCodec>
#include "assembler.h"
#include "lookuptable.h"


int Assem(void)
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
    unsigned short int rs, rt, rd, shamt;
    unsigned int address;
    int immediate;

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

            if(matchId<0) qDebug()<<line<<"Unknown/unsupported instruction name";
            instruction=coreInstructionSet[matchId].opcode<<26
                      | coreInstructionSet[matchId].funct;

            streamXml.readLine();
            switch (matchId) {
            case 0: case 3: case 4: case 16: case 17: case 19: case 22: case 29: case 30:
                //add,addu,and,nor,or,slt,sltu,sub,subu
                rd=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>");
                rs=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>");
                rt=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>");
                instruction=instruction | rs<<21 | rt<<16 | rd<<11;
                break;

            case 1: case 2: case 5: case 18: case 20: case 21:
                //addi,addiu,andi,ori,slti,sltiu
                rt=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>");
                rs=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>");
                immediate=(unsigned short int)matchTable.instructionEncode(streamXml,"<parameter>");
                instruction=instruction | rs<<21 | rt<<16 | immediate;
                break;

            case 6: case 7:
                //beq,bne
                rs=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>");
                rt=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>");
                address=(unsigned short int)matchTable.instructionEncode(streamXml,"<reference>");
                instruction=instruction | rs<<21 | rt<<16 | address;
                break;

            case 8: case 9:
                //j,jal
                address=(unsigned short int)matchTable.instructionEncode(streamXml,"<reference>");
                instruction=instruction | address<<26;
                break;

            case 10:
                //jr
                rs=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>");
                instruction=instruction | rs<<21;
                break;

            case 11: case 12: case 13: case 15: case 25: case 26: case 27: case 28:
                //lbu,lhu,ll,lw,sb,sc,sh,sw
                rt=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>");
                immediate=(unsigned short int)matchTable.instructionEncode(streamXml,"<parameter>");
                rs=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>");
                instruction=instruction | rs<<21 | rt<<16 | immediate;
                break;

            case 14:
                //lui
                rt=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>");
                immediate=(unsigned short int)matchTable.instructionEncode(streamXml,"<parameter>");
                instruction=instruction | rt<<16 | immediate;
                break;

            case 23: case 24:
                //sll,srl
                rd=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>");
                rt=(unsigned short int)matchTable.instructionEncode(streamXml,"<register>");;
                shamt=(unsigned short int)matchTable.instructionEncode(streamXml,"<parameter>");
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
//#endif
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
                    qDebug()<<instructionPattern.capturedTexts();
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

    labelLookUpTable.PrintAll();

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

#if TEST2


int main(void)
{
    QString s;
    qDebug()<<s.length();
    s="wasdsfsafsewrt";
    qDebug()<<s<<"  "<<s.length();

    return 0;
}
#endif
