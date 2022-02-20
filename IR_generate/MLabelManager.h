#ifndef BUAA_COMPILER_MLABELMANAGER_H
#define BUAA_COMPILER_MLABELMANAGER_H
#include <string>

class MLabelManager {
private:
    int whileLabelCnt = 0;
    int ifLabelCnt = 0;
    int shortCircuitCnt = 0;
public:
    std::string getFuncLabel(std::string &name) {
        return name;
    }

    std::string getLabel(const std::string &type) {
        // 耦合度奇高无比
        if (type == "while_begin") {
            return type + "_" + std::to_string(whileLabelCnt++);
        } else if (type == "while_end") {
            return type +  "_" + std::to_string(whileLabelCnt++);
        } else if (type == "else_end") {
            return type +  "_" + std::to_string(ifLabelCnt++);
        } else if (type == "if_end") {
            return type + "_" + std::to_string(ifLabelCnt++);
        } else if (type == "and_exp_false_label_for_short_circuit") {
            // 短路求值的
            return type + "_" + std::to_string(shortCircuitCnt++);
        } else if (type == "cond_end") {
            // 短路求值的
            return type + "_" + std::to_string(shortCircuitCnt++);
        } else {
            throw "MLabelManager::generateLabel: unknown label type";
        }
    }
};

#endif //BUAA_COMPILER_MLABELMANAGER_H
