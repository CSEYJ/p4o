#ifndef BACKENDS_P4O_DATA_DEPENDENCY_ANALYSIS_H_
#define BACKENDS_P4O_DATA_DEPENDENCY_ANALYSIS_H_
#include "frontends/p4/typeMap.h"
#include "frontends/common/resolveReferences/referenceMap.h"
#include "backends/bmv2/simple_switch/simpleSwitch.h"
#include "frontends/p4/cloner.h"
namespace P4O
{
struct DependencyInfo{
    std::vector<const IR::Node *> ordered_control;
    std::vector<const IR::Node *> unordered_control;
    std::set<const IR::Node *> write_list;
    std::set<const IR::Node *> read_list;
};

class DoDataDependencyAnalysis: public Inspector{
    P4::ReferenceMap *refMap;
    P4::TypeMap *typeMap;
    BMV2::V1ProgramStructure *v1arch;
    void sanity(const IR::Node *n)
    {
        if(
            n->is<IR::P4Control>() or
            n->is<IR::BlockStatement>() or
            n->is<IR::P4Table>() or
            n->is<IR::P4Action>() or
            n->is<IR::IfStatement>() or
            n->is<IR::Operation_Binary>() or
            n->is<IR::MethodCallStatement>() or
            n->is<IR::AssignmentStatement>()
        ) return;
        std::cerr << n->node_type_name() << std::endl;
        BUG("not implemented");
    }

public:
    DoDataDependencyAnalysis(
        P4::ReferenceMap *refMap,
        P4::TypeMap *typeMap,
        BMV2::V1ProgramStructure *v1arch
    ) : refMap(refMap),
        typeMap(typeMap),
        v1arch(v1arch) {}
    std::map<const IR::Node *, DependencyInfo *> dependency_map;
    bool preorder(const IR::P4Control *) override;
    bool preorder(const IR::BlockStatement *) override;
    bool preorder(const IR::P4Table *) override;
    bool preorder(const IR::P4Action *) override;
    bool preorder(const IR::IfStatement *) override;
    bool preorder(const IR::MethodCallStatement *) override;
    bool preorder(const IR::AssignmentStatement *) override;
    bool preorder(const IR::P4Parser*) override;
};

class ExpressionBreakdown: public Inspector{
    P4::ReferenceMap *refMap;
    P4::TypeMap *typeMap;
    void sanity(const IR::Node *n){
        if(
            n->is<IR::Operation_Binary>() or
            n->is<IR::MethodCallExpression>() or
            n->is<IR::Constant>() or
            n->is<IR::Member>() or
            n->is<IR::PathExpression>()
        ) return;
        std::cerr << n->node_type_name() << std::endl;
        BUG("not implemented");
    }
public:
    std::set<const IR::Node *> write_list;
    std::set<const IR::Node *> read_list;
    ExpressionBreakdown(
        P4::ReferenceMap *refMap, 
        P4::TypeMap *typeMap
    ): 
        refMap(refMap),
        typeMap(typeMap){}
    bool preorder(const IR::Operation_Binary *) override;
    bool preorder(const IR::MethodCallExpression *) override;
    bool preorder(const IR::Constant *) override;
    bool preorder(const IR::Member *) override;
    bool preorder(const IR::PathExpression *) override;

};

class DataDependencyAnalysis: public PassManager{
public:
    DataDependencyAnalysis(
        P4::ReferenceMap *refMap,
        P4::TypeMap *typeMap,
        BMV2::V1ProgramStructure *v1arch)
    {
        passes.push_back(new DoDataDependencyAnalysis(refMap, typeMap, v1arch));
    }
};

} // namespace P4O
#endif