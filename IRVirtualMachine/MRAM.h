#ifndef BUAA_COMPILER_MRAM_H
#define BUAA_COMPILER_MRAM_H

#include <vector>
#include <stack>

#define self (*this)
#define MAX_TEMP_REGISTER_NUM 1000

class MRAM {
public:
    std::vector<int> *ram;
    int sp; // "sp" for "stack pointer"

    std::vector<int> *spSaver;

    int returnValueRegister;
    int* tempRegisters;

    std::vector<int> *paramStack;

    MRAM() : ram(new std::vector<int>()), sp(0), returnValueRegister(0),
             spSaver(new std::vector<int>()),
             tempRegisters(new int[MAX_TEMP_REGISTER_NUM]()),
             paramStack(new std::vector<int>()){}

    int allocate(int size) {
        int oldSP = self.sp;
        for (; self.sp < oldSP + size; self.sp++)
            self.ram->insert(self.ram->begin() + self.sp, 0);
        return oldSP;
    }

    void push(int value) {
        self.ram->insert(self.ram->begin() + self.sp, value);
        self.sp++;
    }

    void save(int value, int offset) {
        (*(self.ram))[offset] = value;
    }


    int pop() {
        self.sp--;
        return (*(self.ram))[self.sp];
    }

    int load(int offset) {
        return (*(self.ram))[offset];
    }
    int load() {
        return (*(self.ram))[self.sp];
    }

    void savePtr() {
        self.spSaver->push_back(self.sp);
    }

    void loadPtr() {
        self.sp = (*(self.spSaver))[self.spSaver->size() - 1];
        self.spSaver->erase(self.spSaver->end() - 1);
    }

    // tempIR会是"1", "12", "return_value"这样的，没有'%'
    void saveReg(int value, int regID) {
        if (regID == -1) {
            self.returnValueRegister = value;
        } else {
            if (regID >= MAX_TEMP_REGISTER_NUM) {
                throw "MRAM::saveTempIRVar: register no out of boundary";
            }
            self.tempRegisters[regID] = value;
        }
    }

    int loadReg(int regID) {
        if (regID == -1) {
            // -1代表了返回值寄存器
            return self.returnValueRegister;
        } else {
            if (regID >= MAX_TEMP_REGISTER_NUM) {
                throw "MRAM::loadReg: register no out of boundary";
            }
            return self.tempRegisters[regID];
        }
    }

    void pushParam(int value) {
        self.paramStack->push_back(value);
    }

    int popParam() {
        int value = (*(self.paramStack))[self.paramStack->size() - 1];
        self.paramStack->erase(self.paramStack->begin() + self.paramStack->size() - 1);
        return value;
    }

};

#endif //BUAA_COMPILER_MRAM_H
