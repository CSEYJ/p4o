#ifndef BACKENDS_P4O_DATA_DEPENDENCY_ANALYSIS_H_
#define BACKENDS_P4O_DATA_DEPENDENCY_ANALYSIS_H_
#include "frontends/p4/typeMap.h"
#include "frontends/common/resolveReferences/referenceMap.h"
#include "backends/bmv2/simple_switch/simpleSwitch.h"
#include "frontends/p4/cloner.h"
#include <iostream>
#include "lib/json.h"
namespace P4O
{
struct DependencyInfo{
    std::vector<const IR::Node *> ordered_control;
    std::vector<const IR::Node *> unordered_control;
    std::set<const IR::Node *> write_list;
    std::set<const IR::Node *> read_list;
public:
    friend std::ostream& operator<<(std::ostream &out, const DependencyInfo &i){
        out << "ordered_control:" << std::endl;
        for(auto it: i.ordered_control){
            out << it << std::endl;
        }
        out << "unordered_control:" << std::endl;
        for(auto it: i.unordered_control){
            out << it << std::endl;
        }
        out << "write_list:" << std::endl;
        for(auto it: i.write_list){
            out << it << std::endl;
        }
        out << "read_list:" << std::endl;
        for(auto it: i.read_list){
            out << it << std::endl;
        }
        return out;
    }
};

class DataDependencyAnalysis: public Inspector{
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
    DataDependencyAnalysis(
        P4::ReferenceMap *refMap,
        P4::TypeMap *typeMap,
        BMV2::V1ProgramStructure *v1arch
    ) : refMap(refMap),
        typeMap(typeMap),
        v1arch(v1arch) {}
    std::map<const IR::Node *, DependencyInfo *> dependency_map;
    bool preorder(const IR::P4Program *) override;
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
    void simple_expr_sanity(const IR::Node *n){
        if(
            n->is<IR::Constant>() or
            n->is<IR::Member>() or
            n->is<IR::PathExpression>()
        ) return;
        std::cerr << n->node_type_name() << std::endl;
        BUG("not a simple expression");
    }
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

class LiftDependencyToTopLevel: public Inspector{
    std::map<const IR::Node *, DependencyInfo *> *dependency_map;
public:
    const IR::Node **top_level_block;
    LiftDependencyToTopLevel(
        std::map<const IR::Node *, DependencyInfo *> *dependency_map
    ):dependency_map(dependency_map){
        top_level_block = new const IR::Node*;
    }
    bool preorder(const IR::P4Program*) override;
};

class GenerateDependencyGraph: public Inspector{
    std::map<const IR::Node *, DependencyInfo *> *dependency_map;
    std::map<const IR::Node *, DependencyInfo *> *orig_dependency_map;
    int compare(const IR::Node *, const IR::Node *, DependencyInfo *);
    int has_dependency(const IR::Node *, const IR::Node *, Util::JsonArray * variable_shared);
    const int init = 0;
    const int find_table1 = 1;
    const int find_table2 = 2;
    const int before = 3;
    const int after = 4;
    const int dont_care = 5;
    const int dependency = 6;
    const int no_dependency = 7;
    const IR::Node ** top_level_block;
    Util::JsonObject *table_info;
public:
    GenerateDependencyGraph(
        std::map<const IR::Node *, DependencyInfo *> *dependency_map,
        std::map<const IR::Node *, DependencyInfo *> *orig_dependency_map,
        const IR::Node ** top_level_block,
        Util::JsonObject *table_info
    ):
        dependency_map(dependency_map),
        orig_dependency_map(orig_dependency_map),
        top_level_block(top_level_block),
        table_info(table_info){}
    bool preorder(const IR::P4Program* ) override;
        
};

class CollectTableInfo: public Inspector{
    Util::JsonObject *table_info;
    BMV2::V1ProgramStructure *v1arch;
    Util::JsonArray *info;
public:
    CollectTableInfo(
        Util::JsonObject *table_info,
        BMV2::V1ProgramStructure *v1arch
    ): 
        table_info(table_info),
        v1arch(v1arch){
            info = new Util::JsonArray();
            table_info->emplace("info", info);
        }
    bool preorder(const IR::P4Control* ) override;
    bool preorder(const IR::P4Table* ) override;
    void postorder(const IR::P4Program* ) override;
};


class TableDependencyAnalysis: public PassManager{
    Util::JsonObject table_info;
public:
    TableDependencyAnalysis(
        P4::ReferenceMap *refMap,
        P4::TypeMap *typeMap,
        BMV2::V1ProgramStructure*v1arch,
        std::map<const IR::Node *, P4O::DependencyInfo *> *orig_map
    ){
        auto analysis = new DataDependencyAnalysis(refMap, typeMap, v1arch);
        passes.push_back(analysis);
        auto lift = new LiftDependencyToTopLevel(orig_map);
        passes.push_back(lift);
        passes.push_back(new LiftDependencyToTopLevel(&analysis->dependency_map));
        passes.push_back(new GenerateDependencyGraph(
            &analysis->dependency_map, orig_map, lift->top_level_block, &table_info));
        passes.push_back(new CollectTableInfo(&table_info, v1arch));
        
    }

};

} // namespace P4O
#endif