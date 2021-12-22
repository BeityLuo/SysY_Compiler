#ifndef BUAA_COMPILER_IRVAR_H
#define BUAA_COMPILER_IRVAR_H
enum MIRVarTypeID {
    VAR_IR_VAR, REG_IR_VAR, IMM_IR_VAR
};

class MIRVar {
public:
    virtual MIRVarTypeID getTypeID() = 0;

    virtual std::string toString() = 0;
};

class MImmIRVar : public MIRVar {
public:
    int value;

    MImmIRVar(int value) : value(value) {}

    MIRVarTypeID getTypeID() override {
        return IMM_IR_VAR;
    }

    std::string toString() override {
        return std::to_string(value);
    }
};

class MRegIRVar : public MIRVar {
public:
    int regID;

    MRegIRVar(int regId) : regID(regId) {}

    MRegIRVar(std::string name) {
        if (name == "return_value")
            regID = -1;
        else
            throw "MRegIRVar::MRegIRVar: unexpected name: " + name;
    }

    MIRVarTypeID getTypeID() override {
        return REG_IR_VAR;
    }

    std::string toString() override {
        if (regID == -1)
            return "%return_value";
        else
            return '%' + std::to_string(regID);
    }
};

class MVarIRVar : public MIRVar {
private:
    MRegIRVar *regOffset;
    MImmIRVar *immOffset;
public:
    std::string name;

    MVarIRVar(std::string name) : name(name), regOffset(nullptr), immOffset(nullptr) {}
    MVarIRVar(std::string name, MRegIRVar *regOffset)
            : name(name), regOffset(regOffset), immOffset(nullptr) {}

    MVarIRVar(std::string name, MImmIRVar *immOffset)
            : name(name), regOffset(nullptr), immOffset(immOffset) {}

    MIRVarTypeID getTypeID() override {
        return VAR_IR_VAR;
    }

    MIRVar* getOffset() {
        if (regOffset == nullptr) return immOffset;
        else return regOffset;
    }

    std::string toString() override {
        if (regOffset == nullptr && immOffset == nullptr)
            return name;
        else if (regOffset != nullptr)
            return name + '[' + regOffset->toString() + ']';
        else
            return name + '[' + immOffset->toString() + ']';
    }
};


#endif //BUAA_COMPILER_IRVAR_H
