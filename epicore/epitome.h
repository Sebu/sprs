#ifndef EPITOME_H
#define EPITOME_H

#include <vector>
#include "patch.h"

class Epitome
{
public:
    std::vector<Patch*> reconPatches;
    Epitome();

    void grow();

    void save();
};

#endif // EPITOME_H
