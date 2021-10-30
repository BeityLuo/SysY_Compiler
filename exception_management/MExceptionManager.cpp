#include "MExceptionManager.h"

std::vector<MException*>* MExceptionManager::exceptions = new std::vector<MException*>();

void MExceptionManager::pushException(MException* exception) {
    exceptions->push_back(exception);
}
bool MExceptionManager::empty() {
    return exceptions->empty();
}
MException* MExceptionManager::popException() {
    if (!exceptions->empty()) {
        return *(exceptions->begin());
    } else {
        return nullptr;
    }
}
std::string MExceptionManager::toString() {
    std::string ans = "";
    for (auto e : *exceptions) {
        ans += e->toString();
    }
    return ans.substr(0, ans.size() - 1); //去掉最后的换行符
}
