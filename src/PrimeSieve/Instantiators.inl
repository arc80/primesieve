// ply instantiate PrimeSieve
void inst_PrimeSieve(TargetInstantiatorArgs* args) {
    args->buildTarget->targetType = BuildTargetType::EXE;
    args->addSourceFiles(".", false);
    args->addTarget(Visibility::Private, "runtime");
}
