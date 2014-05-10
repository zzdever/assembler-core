#ifndef LOOKUPTABLE_H
#define LOOKUPTABLE_H

#include <QString>
#include <QDebug>

#define MAXLABELAMOUNT 512
#define MAXLABELLENGTH 64

struct labelTable{
    QString name;
    unsigned int address;
};

class LookUpTable
{
private:
    struct labelTable* head;
    int labelAmount;

public:
    LookUpTable();

    void Push(QString name, int addr);
    //char isIn(char *name);
    unsigned int lookUp(QString name);
    void PrintAll(void);
};

#endif // LOOKUPTABLE_H
