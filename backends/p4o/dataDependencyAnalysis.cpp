#include "dataDependencyAnalysis.h"
namespace P4O{
bool DoDataDependencyAnalysis::preorder(const IR::P4Parser*p){
    return false;
}

bool DoDataDependencyAnalysis::preorder(const IR::P4Control *c){
    if(c->name != v1arch->ingress->name) return false;
    auto info = new DependencyInfo;
    dependency_map.emplace(c, info);
    info->ordered_control.push_back(c->body);
    sanity(c->body);
    visit(c->body);
    return false;
}
bool DoDataDependencyAnalysis::preorder(const IR::BlockStatement *b){
    auto info = new DependencyInfo;
    dependency_map.emplace(b, info);
    for(auto stmt: b->components){
        sanity(stmt);
        info->ordered_control.push_back(stmt);
        visit(stmt);
    }
    return false;
}
bool DoDataDependencyAnalysis::preorder(const IR::P4Table *t){
    auto info = new DependencyInfo;
    dependency_map.emplace(t, info);
    for(auto action: t->getActionList()->actionList){
        if(auto mce = action->expression->to<IR::MethodCallExpression>()){
            auto mi = P4::MethodInstance::resolve(mce, refMap, typeMap);
            if(auto action_call = mi->to<P4::ActionCall>()){
                info->unordered_control.push_back(action_call->action);
                sanity(action_call->action);
                visit(action_call->action);
            }
            else{
                BUG("Action Call");
            }
        }
        else{
            BUG("action is not a MethodCallExpression");
        }
    }
    for(auto key: t->getKey()->keyElements){
        info->read_list.insert(key->expression);
    }
    return false;
}
bool DoDataDependencyAnalysis::preorder(const IR::P4Action *a){
    auto info = new DependencyInfo;
    dependency_map.emplace(a, info);
    info->ordered_control.push_back(a->body);
    sanity(a->body);
    visit(a->body);
    return false;
}
bool DoDataDependencyAnalysis::preorder(const IR::IfStatement *i){
    auto info = new DependencyInfo;
    dependency_map.emplace(i, info);
    info->unordered_control.push_back(i->ifTrue);
    info->unordered_control.push_back(i->ifFalse);


    return false;
}

bool DoDataDependencyAnalysis::preorder(const IR::MethodCallStatement *mcs){
    auto info = new DependencyInfo;
    dependency_map.emplace(mcs, info);
    auto mi = P4::MethodInstance::resolve(mcs, refMap, typeMap);
    if (auto apply = mi->to<P4::ApplyMethod>()){
        if (auto table = apply->object->to<IR::P4Table>()){
            info->ordered_control.push_back(table);
            sanity(table);
            visit(table);
        }
        else{
            std::cout << mcs << std::endl;
            BUG("should not happen");
        }
    }
    else {
        std::cerr << mcs << std::endl;
        BUG("not implemented");
    }
    return false;
}

bool DoDataDependencyAnalysis::preorder(const IR::AssignmentStatement *a){
    
    return false;
}

bool ExpressionBreakdown::preorder(const IR::Operation_Binary *bin){
    sanity(bin->left);
    visit(bin->left);
    sanity(bin->right);
    visit(bin->right);
    return false;
}
bool ExpressionBreakdown::preorder(const IR::MethodCallExpression *mce){
    std::cerr << mce << std::endl;
    BUG("to extend");
    return false;
}
bool ExpressionBreakdown::preorder(const IR::Constant *c){
    return false;
}
bool ExpressionBreakdown::preorder(const IR::Member *m){
    read_list.insert(m);
    return false;
}
bool ExpressionBreakdown::preorder(const IR::PathExpression *p){
    read_list.insert(p);
    return false;
}

} //namespace P4O