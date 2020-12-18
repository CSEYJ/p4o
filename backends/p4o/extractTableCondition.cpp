#include "extractTableCondition.h"
namespace P4O{
const IR::Node *BreakIfStatementBlock::postorder(IR::IfStatement* i){
    bool changed = false;
    IR::IndexedVector<IR::StatOrDecl> code_block;
    if(auto true_block = i->ifTrue->to<IR::BlockStatement>()){
        for(auto stmt: true_block->components){
            code_block.push_back(
                new IR::IfStatement(
                    i->condition, stmt->to<IR::Statement>(), nullptr));
        }
        changed = true;
    }
    else if(i->ifTrue->is<IR::MethodCallStatement>() or 
        i->ifTrue->is<IR::IfStatement>()){
        code_block.push_back(
            new IR::IfStatement(i->condition, i->ifTrue, nullptr));
    }
    else if(not i->ifTrue->is<IR::EmptyStatement>() and i->ifTrue){
        std::cerr << i->ifTrue << std::endl; 
        BUG("not implemented");
    }
    if(auto false_block = i->ifFalse->to<IR::BlockStatement>()){
        for(auto stmt: false_block->components){
            code_block.push_back(
                new IR::IfStatement(
                    new IR::LNot(i->condition), 
                    stmt->to<IR::Statement>(), 
                    nullptr
                ));
        }
        changed = true;
    }
    else if(i->ifFalse->is<IR::MethodCallStatement>() or
        i->ifFalse->is<IR::IfStatement>()){
        code_block.push_back(
            new IR::IfStatement(
                new IR::LNot(i->condition), 
                i->ifFalse, 
                nullptr
            ));
        changed = true;
    }
    else if(not i->ifFalse->is<IR::EmptyStatement>() and i->ifFalse){
        std::cerr << i->ifFalse << std::endl; 
        BUG("not implemented");
    }
    if(changed) return new IR::BlockStatement(code_block);
    else return i;
}
const IR::Node *BreakIfStatementBlock::preorder(IR::P4Parser*p){
    prune();
    return p;
}
const IR::Node *BreakIfStatementBlock::preorder(IR::P4Control*c){
    if(v1arch->ingress->name != c->name and c->name != v1arch->egress->name){
        prune();
    }
    return c;
}

const IR::Node *BreakIfStatementBlock::preorder(IR::P4Action *a){
    prune();
    return a;
}


const IR::Node *MergeIfStatement::postorder(IR::IfStatement* i){
    if(i->ifFalse) BUG("BreakIfStatementBlock pass failed");
    if(auto inside_if = i->ifTrue->to<IR::IfStatement>()){
        if(inside_if->ifFalse) BUG("BreakIfStatementBlock pass failed");
        return new IR::IfStatement(
            new IR::LAnd(i->condition, inside_if->condition),
            inside_if->ifTrue,
            nullptr
        );
    }
    return i;
}
const IR::Node *MergeIfStatement::preorder(IR::P4Parser*p){
    prune();
    return p;
}
const IR::Node *MergeIfStatement::preorder(IR::P4Control*c){
    if(v1arch->ingress->name != c->name and c->name != v1arch->egress->name){
        prune();
    }
    return c;
}

const IR::Node *AppendTrueToNonIfStatement::postorder(IR::BlockStatement* b){
    prune();
    IR::IndexedVector<IR::StatOrDecl> code_block;
    for(auto stmt: b->components){
        if(not stmt->is<IR::IfStatement>()){
            code_block.push_back(new IR::IfStatement(
                new IR::BoolLiteral(true), 
                stmt->to<IR::Statement>(), 
                nullptr
            ));
        }
        else code_block.push_back(stmt);
    }
    return new IR::BlockStatement(code_block);
}
const IR::Node *AppendTrueToNonIfStatement::preorder(IR::P4Parser*p){
    prune();
    return p;
}
const IR::Node *AppendTrueToNonIfStatement::preorder(IR::P4Control*c){
    if(v1arch->ingress->name != c->name and c->name != v1arch->egress->name){
        prune();
    }
    return c;
}

const IR::Node *AppendTrueToNonIfStatement::preorder(IR::P4Action *a){
    prune();
    return a;
}

} //namespace P4o