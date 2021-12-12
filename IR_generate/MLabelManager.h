#ifndef BUAA_COMPILER_MLABELMANAGER_H
#define BUAA_COMPILER_MLABELMANAGER_H
#include <string>

class MLabelManager {
private:
    int whileLabelCnt = 0;
    int ifEndLabelCnt = 0;
    int elseEndLabelCnt = 0;
public:
    std::string getFuncLabel(std::string &name) {
        return name;
    }

    std::string getLabel(std::string &type) {
        // 耦合度奇高无比
        if (type == "while_begin") {
            return type + "_" + std::to_string(whileLabelCnt);
        } else if (type == "while_end") {
            return type +  "_" + std::to_string(whileLabelCnt++);
        } else if (type == "else_end") {
            return type +  "_" + std::to_string(elseEndLabelCnt);
        } else if (type == "if_end") {
            self.elseEndLabelCnt++;
            return type + "_" + std::to_string(ifEndLabelCnt++);
        } else {
            throw "MLabelManager::generateLabel: unknown label type";
        }
    }
};

#endif //BUAA_COMPILER_MLABELMANAGER_H
