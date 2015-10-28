package phoenix

// #include "go-clang.h"
import "C"

import (
	"unsafe"
)

// An "index" that consists of a set of translation units that would typically be linked together into an executable or library.
type Index struct {
	c C.CXIndex
}

/*
 * \brief Provides a shared context for creating translation units.
 *
 * It provides two options:
 *
 * - excludeDeclarationsFromPCH: When non-zero, allows enumeration of "local"
 * declarations (when loading any new translation units). A "local" declaration
 * is one that belongs in the translation unit itself and not in a precompiled
 * header that was used by the translation unit. If zero, all declarations
 * will be enumerated.
 *
 * Here is an example:
 *
 * \code
 *   // excludeDeclsFromPCH = 1, displayDiagnostics=1
 *   Idx = clang_createIndex(1, 1);
 *
 *   // IndexTest.pch was produced with the following command:
 *   // "clang -x c IndexTest.h -emit-ast -o IndexTest.pch"
 *   TU = clang_createTranslationUnit(Idx, "IndexTest.pch");
 *
 *   // This will load all the symbols from 'IndexTest.pch'
 *   clang_visitChildren(clang_getTranslationUnitCursor(TU),
 *                       TranslationUnitVisitor, 0);
 *   clang_disposeTranslationUnit(TU);
 *
 *   // This will load all the symbols from 'IndexTest.c', excluding symbols
 *   // from 'IndexTest.pch'.
 *   char *args[] = { "-Xclang", "-include-pch=IndexTest.pch" };
 *   TU = clang_createTranslationUnitFromSourceFile(Idx, "IndexTest.c", 2, args,
 *                                                  0, 0);
 *   clang_visitChildren(clang_getTranslationUnitCursor(TU),
 *                       TranslationUnitVisitor, 0);
 *   clang_disposeTranslationUnit(TU);
 * \endcode
 *
 * This process of creating the 'pch', loading it separately, and using it (via
 * -include-pch) allows 'excludeDeclsFromPCH' to remove redundant callbacks
 * (which gives the indexer the same performance benefit as the compiler).
 */
func NewIndex(excludeDeclarationsFromPCH int16, displayDiagnostics int16) Index {
	return Index{C.clang_createIndex(C.int(excludeDeclarationsFromPCH), C.int(displayDiagnostics))}
}

// Destroy the given index. The index must not be destroyed until all of the translation units created within that index have been destroyed.
func (i Index) Dispose() {
	C.clang_disposeIndex(i.c)
}

/*
 * \brief Sets general options associated with a CXIndex.
 *
 * For example:
 * \code
 * CXIndex idx = ...;
 * clang_CXIndex_setGlobalOptions(idx,
 *     clang_CXIndex_getGlobalOptions(idx) |
 *     CXGlobalOpt_ThreadBackgroundPriorityForIndexing);
 * \endcode
 *
 * \param options A bitmask of options, a bitwise OR of CXGlobalOpt_XXX flags.
 */
func (i Index) SetGlobalOptions(options uint16) {
	C.clang_CXIndex_setGlobalOptions(i.c, C.uint(options))
}

// Gets the general options associated with a CXIndex. \returns A bitmask of options, a bitwise OR of CXGlobalOpt_XXX flags that are associated with the given CXIndex object.
func (i Index) GlobalOptions() uint16 {
	return uint16(C.clang_CXIndex_getGlobalOptions(i.c))
}

// Return the CXTranslationUnit for a given source file and the provided command line arguments one would pass to the compiler. Note: The 'source_filename' argument is optional. If the caller provides a NULL pointer, the name of the source file is expected to reside in the specified command line arguments. Note: When encountered in 'clang_command_line_args', the following options are ignored: '-c' '-emit-ast' '-fsyntax-only' '-o \<output file>' (both '-o' and '\<output file>' are ignored) \param CIdx The index object with which the translation unit will be associated. \param source_filename The name of the source file to load, or NULL if the source file is included in \p clang_command_line_args. \param num_clang_command_line_args The number of command-line arguments in \p clang_command_line_args. \param clang_command_line_args The command-line arguments that would be passed to the \c clang executable if it were being invoked out-of-process. These command-line options will be parsed and will affect how the translation unit is parsed. Note that the following options are ignored: '-c', '-emit-ast', '-fsyntax-only' (which is the default), and '-o \<output file>'. \param num_unsaved_files the number of unsaved file entries in \p unsaved_files. \param unsaved_files the files that have not yet been saved to disk but may be required for code completion, including the contents of those files. The contents and name of these files (as specified by CXUnsavedFile) are copied when necessary, so the client only needs to guarantee their validity until the call to this function returns.
func (i Index) TranslationUnitFromSourceFile(source_filename string, clang_command_line_args []string, unsaved_files []UnsavedFile) TranslationUnit {
	ca_clang_command_line_args := make([]*C.char, len(clang_command_line_args))
	for i := range clang_command_line_args {
		ci_str := C.CString(clang_command_line_args[i])
		defer C.free(unsafe.Pointer(ci_str))
		ca_clang_command_line_args[i] = ci_str
	}
	ca_unsaved_files := make([]C.struct_CXUnsavedFile, len(unsaved_files))
	for i := range unsaved_files {
		ca_unsaved_files[i] = unsaved_files[i].c
	}

	c_source_filename := C.CString(source_filename)
	defer C.free(unsafe.Pointer(c_source_filename))

	return TranslationUnit{C.clang_createTranslationUnitFromSourceFile(i.c, c_source_filename, C.int(len(clang_command_line_args)), &ca_clang_command_line_args[0], C.uint(len(unsaved_files)), &ca_unsaved_files[0])}
}

// Create a translation unit from an AST file (-emit-ast).
func (i Index) TranslationUnit(ast_filename string) TranslationUnit {
	c_ast_filename := C.CString(ast_filename)
	defer C.free(unsafe.Pointer(c_ast_filename))

	return TranslationUnit{C.clang_createTranslationUnit(i.c, c_ast_filename)}
}

// Parse the given source file and the translation unit corresponding to that file. This routine is the main entry point for the Clang C API, providing the ability to parse a source file into a translation unit that can then be queried by other functions in the API. This routine accepts a set of command-line arguments so that the compilation can be configured in the same way that the compiler is configured on the command line. \param CIdx The index object with which the translation unit will be associated. \param source_filename The name of the source file to load, or NULL if the source file is included in \p command_line_args. \param command_line_args The command-line arguments that would be passed to the \c clang executable if it were being invoked out-of-process. These command-line options will be parsed and will affect how the translation unit is parsed. Note that the following options are ignored: '-c', '-emit-ast', '-fsyntax-only' (which is the default), and '-o \<output file>'. \param num_command_line_args The number of command-line arguments in \p command_line_args. \param unsaved_files the files that have not yet been saved to disk but may be required for parsing, including the contents of those files. The contents and name of these files (as specified by CXUnsavedFile) are copied when necessary, so the client only needs to guarantee their validity until the call to this function returns. \param num_unsaved_files the number of unsaved file entries in \p unsaved_files. \param options A bitmask of options that affects how the translation unit is managed but not its compilation. This should be a bitwise OR of the CXTranslationUnit_XXX flags. \returns A new translation unit describing the parsed code and containing any diagnostics produced by the compiler. If there is a failure from which the compiler cannot recover, returns NULL.
func (i Index) ParseTranslationUnit(source_filename string, command_line_args []string, unsaved_files []UnsavedFile, options uint16) TranslationUnit {
	ca_command_line_args := make([]*C.char, len(command_line_args))
	for i := range command_line_args {
		ci_str := C.CString(command_line_args[i])
		defer C.free(unsafe.Pointer(ci_str))
		ca_command_line_args[i] = ci_str
	}
	ca_unsaved_files := make([]C.struct_CXUnsavedFile, len(unsaved_files))
	for i := range unsaved_files {
		ca_unsaved_files[i] = unsaved_files[i].c
	}

	c_source_filename := C.CString(source_filename)
	defer C.free(unsafe.Pointer(c_source_filename))

	return TranslationUnit{C.clang_parseTranslationUnit(i.c, c_source_filename, &ca_command_line_args[0], C.int(len(command_line_args)), &ca_unsaved_files[0], C.uint(len(unsaved_files)), C.uint(options))}
}

// An indexing action/session, to be applied to one or multiple translation units. \param CIdx The index object with which the index action will be associated.
func (i Index) Action_create() IndexAction {
	return IndexAction{C.clang_IndexAction_create(i.c)}
}
