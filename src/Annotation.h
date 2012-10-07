#include <llvm/Module.h>
#include <llvm/Type.h>
#include <llvm/Instructions.h>
#include <llvm/Metadata.h>
#include <llvm/Support/Path.h>
#include <string>
#include <llvm/Support/Debug.h>

static inline bool isFunctionPointer(llvm::Type *Ty) {
	llvm::PointerType *PTy = llvm::dyn_cast<llvm::PointerType>(Ty);
	return PTy && PTy->getElementType()->isFunctionTy();
}

static inline std::string getScopeName(llvm::GlobalValue *GV) {
	if (llvm::GlobalValue::isExternalLinkage(GV->getLinkage()))
		return GV->getName();
	else {
		llvm::StringRef moduleName = llvm::sys::path::stem(
			GV->getParent()->getModuleIdentifier());
		return "_" + moduleName.str() + "." + GV->getName().str();
	}
}

// prefix anonymous struct name with module name
static inline std::string getScopeName(llvm::Type *Ty, llvm::Module *M) {
	if (Ty->getStructName().startswith("struct.anon")) {
		llvm::StringRef rest = Ty->getStructName().substr(6);
		llvm::StringRef moduleName = llvm::sys::path::stem(
			M->getModuleIdentifier());
		return "struct._" + moduleName.str() + rest.str();
	}
	return Ty->getStructName().str();
}

static inline llvm::StringRef getLoadStoreId(llvm::Instruction *I) {
	if (llvm::MDNode *MD = I->getMetadata("id"))
		return llvm::dyn_cast<llvm::MDString>(MD->getOperand(0))->getString();
	return llvm::StringRef();
}

static inline std::string
getStructId(llvm::Type *Ty, llvm::Module *M, unsigned offset) {
	return getScopeName(Ty, M) + "." + llvm::Twine(offset).str();
}

static inline std::string getVarId(llvm::GlobalValue *GV) {
	return "var." + getScopeName(GV);
}

static inline std::string getArgId(llvm::Argument *A) {
	return "arg." + getScopeName(A->getParent()) + "."
			+ llvm::Twine(A->getArgNo()).str();
}

static inline std::string getArgId(llvm::Function *F, unsigned no) {
	return "arg." + getScopeName(F) + "." + llvm::Twine(no).str();
}

static inline std::string getRetId(llvm::Function *F) {
	return "ret." + getScopeName(F);
}

static inline std::string getValueId(llvm::Value *V);
static inline std::string getRetId(llvm::CallInst *CI) {
	if (llvm::Function *CF = CI->getCalledFunction())
		return getRetId(CF);
	else {
		std::string sID = getValueId(CI->getCalledValue());
		if (sID != "")
			return "ret." + sID;
	}
	return "";
}

static inline std::string getValueId(llvm::Value *V) {
	if (llvm::Argument *A = llvm::dyn_cast<llvm::Argument>(V))
		return getArgId(A);
	else if (llvm::CallInst *CI = llvm::dyn_cast<llvm::CallInst>(V))
		return getRetId(CI);
	else if (llvm::isa<llvm::LoadInst>(V) || llvm::isa<llvm::StoreInst>(V))
		return getLoadStoreId(llvm::dyn_cast<llvm::Instruction>(V));
	return "";
}

