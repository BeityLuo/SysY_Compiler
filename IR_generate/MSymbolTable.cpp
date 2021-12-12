#include "MSymbolTable.h"
#include "MSymbolTableItem.h"

void MSymbolTable::addSymbolItem(MSymbolTableItem *symbolTableItem) {
    self.table[symbolTableItem->getSymbol()] = symbolTableItem;
}


