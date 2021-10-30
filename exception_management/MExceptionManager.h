#ifndef BUAA_COMPILER_MEXCEPTIONMANAGER_H
#define BUAA_COMPILER_MEXCEPTIONMANAGER_H

#include <vector>
#include "exceptions.h"

class MExceptionManager {
private:
    static std::vector<MException*>* exceptions;
    MExceptionManager() {}
public:
    static void pushException(MException* exception);
    static bool empty();
    static MException* popException();
    static std::string toString();
};

#endif //BUAA_COMPILER_MEXCEPTIONMANAGER_H
