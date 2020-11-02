#include "backend.h"
namespace P4O{
    void P4OBackend::analysis(const IR::ToplevelBlock *tlb)
    {
        auto structure = new BMV2::V1ProgramStructure();

        auto parseV1Arch = new BMV2::ParseV1Architecture(structure);
        auto main = tlb->getMain();
        if (!main)
            return; // no main

        if (main->type->name != "V1Switch")
            ::warning(ErrorType::WARN_INVALID, "%1%: the main package should be called V1Switch"
                                               "; are you using the wrong architecture?",
                      main->type->name);

        main->apply(*parseV1Arch);
        if (::errorCount() > 0)
            return;

        auto evaluator = new P4::EvaluatorPass(refMap, typeMap);
        auto program = tlb->getProgram();
        // These passes are logically bmv2-specific
        PassManager simplify = {};

        simplify.addPasses({
            new P4::ClearTypeMap(typeMap), // because the user metadata type has changed
            new P4::SynthesizeActions(refMap, typeMap,
                                      new BMV2::SkipControls(&structure->non_pipeline_controls)),
            new P4::MoveActionsToTables(refMap, typeMap),
            new P4::TypeChecking(refMap, typeMap),
            new P4::SimplifyControlFlow(refMap, typeMap),
            new BMV2::LowerExpressions(typeMap),
            new P4::ConstantFolding(refMap, typeMap, false),
            new P4::TypeChecking(refMap, typeMap),
            new BMV2::RemoveComplexExpressions(refMap, typeMap,
                                         new BMV2::ProcessControls(&structure->pipeline_controls)),
            new P4::SimplifyControlFlow(refMap, typeMap),
            new P4::RemoveAllUnusedDeclarations(refMap),
            // Converts the DAG into a TREE (at least for expressions)
            // This is important later for conversion to JSON.
            new P4::ClonePathExpressions(),
            new P4::ClearTypeMap(typeMap),
            evaluator,
            new VisitFunctor([this, evaluator]() { toplevel = evaluator->getToplevelBlock(); }),
        });
        program = program->apply(simplify);
        auto analysis_orig = new DataDependencyAnalysis(
            refMap, typeMap, structure);
        program->apply(*analysis_orig);
        // for(auto it: analysis_orig->dependency_map){
        //     std::cerr << it.first << std::endl;
        //     std::cerr << *it.second << std::endl;
        // }
        auto new_program = 
            program->apply(ExtractTableCondition(refMap, typeMap, structure));
        // std::cerr << new_program << std::endl;
        auto analysis_new = new TableDependencyAnalysis(
            refMap, typeMap, structure, &analysis_orig->dependency_map);
        new_program->apply(*analysis_new);

    }

} //namespace P4O