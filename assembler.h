#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <iostream>
#include <QString>
#include <QRegExp>
#include <QTextStream>
#include "lookuptable.h"


struct CoreInstruction{
    char mnemonic[7];
    short int opcode;
    short int funct;
};

struct Register{
    char name[5];
    short int number;
};

#define INSTRUCTIONSETSIZE 31
#define REGISTERSETSIZE 32
//short int coreInstructionSetIndex[26];

static struct CoreInstruction coreInstructionSet[]={
    {"add", 0, 0x20},
    {"addi", 0x8, 0},
    {"addiu", 0x9, 0},
    {"addu", 0, 0x21},
    {"and", 0, 0x24},
    {"andi", 0xC, 0},
    {"beq", 0x4, 0},
    {"bne", 0x5, 0},
    {"j", 0x2, 0},
    {"jal", 0x3, 0},
    {"jr", 0, 0x8},
    {"lbu", 0x24, 0},
    {"lhu", 0x25, 0},
    {"ll", 0x30, 0},
    {"lui", 0xF, 0},
    {"lw", 0x23, 0},
    {"nor", 0, 0x27},
    {"or", 0, 0x25},
    {"ori", 0xD, 0},
    {"slt", 0, 0x2A},
    {"slti", 0xA, 0},
    {"sltiu", 0xB, 0},
    {"sltu", 0, 0x2B},
    {"sll", 0, 0},
    {"srl", 0, 0x2},
    {"sb", 0x28, 0},
    {"sc", 0x38, 0},
    {"sh", 0x29, 0},
    {"sw", 0x2B, 0},
    {"sub", 0, 0x22},
    {"subu", 0, 0x23}
};

static struct Register registerSet[]={
    {"zero", 0},
    {"at", 1},
    {"v0", 2},
    {"v1", 3},
    {"a0", 4},
    {"a1", 5},
    {"a2", 6},
    {"a3", 7},
    {"t0", 8},
    {"t1", 9},
    {"t2", 10},
    {"t3", 11},
    {"t4", 12},
    {"t5", 13},
    {"t6", 14},
    {"t7", 15},
    {"s0", 16},
    {"s1", 17},
    {"s2", 18},
    {"s3", 19},
    {"s4", 20},
    {"s5", 21},
    {"s6", 22},
    {"s7", 23},
    {"t8", 24},
    {"t9", 25},
    {"k0", 26},
    {"k1", 27},
    {"gp", 28},
    {"sp", 29},
    {"fp", 30},
    {"ra", 31}
};


class MatchTable {
private:
    QRegExp registerPatternName;
    QRegExp registerPatternNumber;
    QString line;

    int hexTextToInt(QString);

public:
    MatchTable(void);
    int MatchInstruction(QString);
    int MatchRegister(QString);
    int DisassemMatchRegister(unsigned int, unsigned int);
    int instructionEncode(QTextStream &streamXml, QString type, LookUpTable labelTable);
};

#endif // ASSEMBLER_H
