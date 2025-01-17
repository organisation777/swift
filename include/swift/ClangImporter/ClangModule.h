//===--- ClangModule.h - An imported Clang module ---------------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
// This file implements support for loading Clang modules into Swift.
//
//===----------------------------------------------------------------------===//
#ifndef SWIFT_CLANGIMPORTER_CLANGMODULE_H
#define SWIFT_CLANGIMPORTER_CLANGMODULE_H

#include "swift/AST/Module.h"
#include "swift/ClangImporter/ClangImporter.h"
#include "clang/AST/ExternalASTSource.h"

namespace clang {
  class ASTContext;
  class Module;
}

namespace swift {

class ASTContext;
class ModuleLoader;

/// Represents a Clang module that has been imported into Swift.
class ClangModuleUnit final : public LoadedFile {
  ClangImporter::Implementation &owner;
  const clang::Module *clangModule;
  llvm::PointerIntPair<ModuleDecl *, 1, bool> overlayModule;
  mutable Optional<ArrayRef<ModuleDecl::ImportedModule>> importedModulesForLookup;
  /// The metadata of the underlying Clang module.
  clang::ExternalASTSource::ASTSourceDescriptor ASTSourceDescriptor;

  ~ClangModuleUnit() = default;

public:
  /// True if the given Module contains an imported Clang module unit.
  static bool hasClangModule(ModuleDecl *M);

  ClangModuleUnit(ModuleDecl &M, ClangImporter::Implementation &owner,
                  const clang::Module *clangModule);

  /// Retrieve the underlying Clang module.
  ///
  /// This will be null if the module unit represents the imported headers.
  const clang::Module *getClangModule() const { return clangModule; }

  /// Returns true if this is a top-level Clang module (not a submodule).
  bool isTopLevel() const;

  /// Returns the Swift module that overlays this Clang module.
  ModuleDecl *getOverlayModule() const override;

  /// Retrieve the "exported" name of the module, which is usually the module
  /// name, but might be the name of the public module through which this
  /// (private) module is re-exported.
  StringRef getExportedModuleName() const override;

  virtual bool isSystemModule() const override;

  virtual void lookupValue(ModuleDecl::AccessPathTy accessPath,
                           DeclName name, NLKind lookupKind,
                           SmallVectorImpl<ValueDecl*> &results) const override;

  virtual TypeDecl *
  lookupNestedType(Identifier name,
                   const NominalTypeDecl *baseType) const override;

  virtual void lookupVisibleDecls(ModuleDecl::AccessPathTy accessPath,
                                  VisibleDeclConsumer &consumer,
                                  NLKind lookupKind) const override;

  virtual void lookupClassMembers(ModuleDecl::AccessPathTy accessPath,
                                  VisibleDeclConsumer &consumer) const override;

  virtual void
  lookupClassMember(ModuleDecl::AccessPathTy accessPath, DeclName name,
                    SmallVectorImpl<ValueDecl*> &decls) const override;

  void lookupObjCMethods(
         ObjCSelector selector,
         SmallVectorImpl<AbstractFunctionDecl *> &results) const override;

  virtual void getTopLevelDecls(SmallVectorImpl<Decl*> &results) const override;

  virtual void getDisplayDecls(SmallVectorImpl<Decl*> &results) const override;

  virtual void
  getImportedModules(SmallVectorImpl<ModuleDecl::ImportedModule> &imports,
                     ModuleDecl::ImportFilter filter) const override;

  virtual void getImportedModulesForLookup(
      SmallVectorImpl<ModuleDecl::ImportedModule> &imports) const override;

  virtual void
  collectLinkLibraries(ModuleDecl::LinkLibraryCallback callback) const override;

  Identifier
  getDiscriminatorForPrivateValue(const ValueDecl *D) const override {
    llvm_unreachable("no private decls in Clang modules");
  }

  virtual StringRef getFilename() const override;

  virtual const clang::Module *getUnderlyingClangModule() const override {
    return getClangModule();
  }

  clang::ASTContext &getClangASTContext() const;

  /// Returns the ASTSourceDescriptor of the associated Clang module if one
  /// exists.
  Optional<clang::ExternalASTSource::ASTSourceDescriptor>
  getASTSourceDescriptor() const;

  static bool classof(const FileUnit *file) {
    return file->getKind() == FileUnitKind::ClangModule;
  }
  static bool classof(const DeclContext *DC) {
    return isa<FileUnit>(DC) && classof(cast<FileUnit>(DC));
  }
};

}

#endif
