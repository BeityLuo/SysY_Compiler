#ifndef BUAA_COMPILER_IRVAR_H
#define BUAA_COMPILER_IRVAR_H
enum MIRVarTypeID {
    VAR_IR_VAR, REG_IR_VAR, IMM_IR_VAR
};

class MIRVar {
public:
    virtual MIRVarTypeID getTypeID() = 0;
};

class MImmIRVar : public MIRVar {
public:
    int value;

    MImmIRVar(int value) : value(value) {}

    MIRVarTypeID getTypeID() override {
        return IMM_IR_VAR;
    }
};

class MVarIRVar : public MIRVar {
public:
    std::string name;
    MImmIRVar *offset;

    MVarIRVar(const std::string name, MImmIRVar *offset)
            : name(name), offset(offset) {}

    MIRVarTypeID getTypeID() override {
        return VAR_IR_VAR;
    }
};

class MRegIRVar : public MIRVar {
public:
    int regID;

    MRegIRVar(int regId) : regID(regId) {}

    MIRVarTypeID getTypeID() override {
        return REG_IR_VAR;
    }
};


#endif //BUAA_COMPILER_IRVAR_H
