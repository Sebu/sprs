#include "dictionary.h"

Dictionary::Dictionary()
{


}

DEntry* Dictionary::getFirst() {
    return elements_.front();
}
