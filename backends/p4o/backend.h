#ifndef BACKENDS_P4O_BACKEND_H_
#define BACKENDS_P4O_BACKEND_H_
#include "frontends/p4/typeMap.h"
#include "frontends/common/resolveReferences/referenceMap.h"
#include "backends/bmv2/simple_switch/simpleSwitch.h"
#include "frontends/p4/cloner.h"
#include "dataDependencyAnalysis.h"
#include "extractTableCondition.h"
namespace P4O{
class P4OBackend{
    const IR::ToplevelBlock *toplevel;
    P4::ReferenceMap *refMap;
    P4::TypeMap *typeMap;
public:
    P4OBackend(
        P4::ReferenceMap *refMap, 
        P4::TypeMap *typeMap
    ):
        refMap(refMap), 
        typeMap(typeMap) {}
    void analysis(const IR::ToplevelBlock *);
};
} // namespace P4O
#endif