#ifndef BACKENDS_P4O_EXTRACT_TABLE_CONDITION_H_
#define BACKENDS_P4O_EXTRACT_TABLE_CONDITION_H_
#include "frontends/p4/typeMap.h"
#include "frontends/common/resolveReferences/referenceMap.h"
#include "backends/bmv2/simple_switch/simpleSwitch.h"
#include "frontends/p4/cloner.h"
#include "frontends/p4/simplify.h"
namespace P4O
{
class BreakIfStatementBlock:public Transform{
    BMV2::V1ProgramStructure *v1arch;
public:
    BreakIfStatementBlock(BMV2::V1ProgramStructure *v1arch):v1arch(v1arch){}
    const IR::Node *postorder(IR::IfStatement*) override;
    const IR::Node *preorder(IR::P4Parser*) override;
    const IR::Node *preorder(IR::P4Control*) override;
    const IR::Node *preorder(IR::P4Action *) override;
};

class MergeIfStatement: public Transform{
    BMV2::V1ProgramStructure *v1arch;
public:
    MergeIfStatement(BMV2::V1ProgramStructure *v1arch): v1arch(v1arch){}
    const IR::Node *postorder(IR::IfStatement*) override;
    const IR::Node *preorder(IR::P4Parser*) override;
    const IR::Node *preorder(IR::P4Control*) override;
};
class AppendTrueToNonIfStatement: public Transform{
    BMV2::V1ProgramStructure *v1arch;
public:
    AppendTrueToNonIfStatement(BMV2::V1ProgramStructure *v1arch): v1arch(v1arch){}
    const IR::Node *postorder(IR::BlockStatement*) override;
    const IR::Node *preorder(IR::P4Parser*) override;
    const IR::Node *preorder(IR::P4Control*) override;
    const IR::Node *preorder(IR::P4Action *) override;
};

class PrintProgram: public Inspector{
public:
    PrintProgram(){}
    bool preorder(const IR::P4Program *p) override{
        for(auto n : p->objects){
            if(n->is<IR::P4Control>()){
                std::cerr << n << std::endl<< std::endl<< std::endl;
            }
        }
        return false;
    }
};

class ExtractTableCondition: public PassManager{
public:
    ExtractTableCondition(
        P4::ReferenceMap *refMap,
        P4::TypeMap *typeMap,
        BMV2::V1ProgramStructure *v1arch
        ){
        auto repeated = new PassRepeated();
        repeated->addPasses({
            new BreakIfStatementBlock(v1arch),
            new P4::SimplifyControlFlow(refMap, typeMap)
        });

        passes.push_back(repeated);
        // passes.push_back(new PrintProgram());
        passes.push_back(new MergeIfStatement(v1arch));
        passes.push_back(new AppendTrueToNonIfStatement(v1arch));
    }
};

} // namespace P4O
#endif