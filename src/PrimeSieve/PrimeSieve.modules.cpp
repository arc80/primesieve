#include <ply-build-repo/Module.h>

// [ply module="PrimeSieve"]
void module_PrimeSieve(ModuleArgs* args) {
    args->buildTarget->targetType = BuildTargetType::EXE;
    args->addSourceFiles(".", false);
    args->addTarget(Visibility::Private, "runtime");
}
