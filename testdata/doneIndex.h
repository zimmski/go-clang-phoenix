
/**
 * \brief Provides the contents of a file that has not yet been saved to disk.
 *
 * Each CXUnsavedFile instance provides the name of a file on the
 * system along with the current contents of that file that have not
 * yet been saved to disk.
 */
struct CXUnsavedFile {
  /**
   * \brief The file whose contents have not yet been saved.
   *
   * This file must already exist in the file system.
   */
  const char *Filename;

  /**
   * \brief A buffer containing the unsaved contents of this file.
   */
  const char *Contents;

  /**
   * \brief The length of the unsaved contents of this buffer.
   */
  unsigned long Length;
};

/**
 * \brief Describes a version number of the form major.minor.subminor.
 */
typedef struct CXVersion {
  /**
   * \brief The major version number, e.g., the '10' in '10.7.3'. A negative
   * value indicates that there is no version number at all.
   */
  int Major;
  /**
   * \brief The minor version number, e.g., the '7' in '10.7.3'. This value
   * will be negative if no minor version number was provided, e.g., for
   * version '10'.
   */
  int Minor;
  /**
   * \brief The subminor version number, e.g., the '3' in '10.7.3'. This value
   * will be negative if no minor or subminor version number was provided,
   * e.g., in version '10' or '10.7'.
   */
  int Subminor;
} CXVersion;

/**
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
CXIndex clang_createIndex(int excludeDeclarationsFromPCH,
                                         int displayDiagnostics);

/**
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
void clang_CXIndex_setGlobalOptions(CXIndex, unsigned options);

/**
 * \brief Retrieve the last modification time of the given file.
 */
time_t clang_getFileTime(CXFile SFile);

/**
 * \brief Uniquely identifies a CXFile, that refers to the same underlying file,
 * across an indexing session.
 */
typedef struct {
  unsigned long long data[3];
} CXFileUniqueID;

/**
 * \brief Retrieve the unique ID for the given \c file.
 *
 * \param file the file to get the ID for.
 * \param outID stores the returned CXFileUniqueID.
 * \returns If there was a failure getting the unique ID, returns non-zero,
 * otherwise returns 0.
*/
int clang_getFileUniqueID(CXFile file, CXFileUniqueID *outID);

/**
 * \brief Determine whether the given header is guarded against
 * multiple inclusions, either with the conventional
 * \#ifndef/\#define/\#endif macro guards or with \#pragma once.
 */
unsigned
clang_isFileMultipleIncludeGuarded(CXTranslationUnit tu, CXFile file);

/**
 * \brief Retrieve a file handle within the given translation unit.
 *
 * \param tu the translation unit
 *
 * \param file_name the name of the file.
 *
 * \returns the file handle for the named file in the translation unit \p tu,
 * or a NULL file handle if the file was not a part of this translation unit.
 */
CXFile clang_getFile(CXTranslationUnit tu,
                                    const char *file_name);

/**
 * \brief Identifies a specific source location within a translation
 * unit.
 *
 * Use clang_getExpansionLocation() or clang_getSpellingLocation()
 * to map a source location to a particular file, line, and column.
 */
typedef struct {
  const void *ptr_data[2];
  unsigned int_data;
} CXSourceLocation;

/**
 * \brief Identifies a half-open character range in the source code.
 *
 * Use clang_getRangeStart() and clang_getRangeEnd() to retrieve the
 * starting and end locations from a source range, respectively.
 */
typedef struct {
  const void *ptr_data[2];
  unsigned begin_int_data;
  unsigned end_int_data;
} CXSourceRange;

/**
 * \brief Retrieves the source location associated with a given file/line/column
 * in a particular translation unit.
 */
CXSourceLocation clang_getLocation(CXTranslationUnit tu,
                                                  CXFile file,
                                                  unsigned line,
                                                  unsigned column);
/**
 * \brief Retrieves the source location associated with a given character offset
 * in a particular translation unit.
 */
CXSourceLocation clang_getLocationForOffset(CXTranslationUnit tu,
                                                           CXFile file,
                                                           unsigned offset);

/**
 * \brief Retrieve a source range given the beginning and ending source
 * locations.
 */
CXSourceRange clang_getRange(CXSourceLocation begin,
                                            CXSourceLocation end);

/**
 * \brief Retrieve the file, line, column, and offset represented by
 * the given source location.
 *
 * If the location refers into a macro expansion, retrieves the
 * location of the macro expansion.
 *
 * \param location the location within a source file that will be decomposed
 * into its parts.
 *
 * \param file [out] if non-NULL, will be set to the file to which the given
 * source location points.
 *
 * \param line [out] if non-NULL, will be set to the line to which the given
 * source location points.
 *
 * \param column [out] if non-NULL, will be set to the column to which the given
 * source location points.
 *
 * \param offset [out] if non-NULL, will be set to the offset into the
 * buffer to which the given source location points.
 */
void clang_getExpansionLocation(CXSourceLocation location,
                                               CXFile *file,
                                               unsigned *line,
                                               unsigned *column,
                                               unsigned *offset);

/**
 * \brief Retrieve the file, line, column, and offset represented by
 * the given source location, as specified in a # line directive.
 *
 * Example: given the following source code in a file somefile.c
 *
 * \code
 * #123 "dummy.c" 1
 *
 * static int func(void)
 * {
 *     return 0;
 * }
 * \endcode
 *
 * the location information returned by this function would be
 *
 * File: dummy.c Line: 124 Column: 12
 *
 * whereas clang_getExpansionLocation would have returned
 *
 * File: somefile.c Line: 3 Column: 12
 *
 * \param location the location within a source file that will be decomposed
 * into its parts.
 *
 * \param filename [out] if non-NULL, will be set to the filename of the
 * source location. Note that filenames returned will be for "virtual" files,
 * which don't necessarily exist on the machine running clang - e.g. when
 * parsing preprocessed output obtained from a different environment. If
 * a non-NULL value is passed in, remember to dispose of the returned value
 * using \c clang_disposeString() once you've finished with it. For an invalid
 * source location, an empty string is returned.
 *
 * \param line [out] if non-NULL, will be set to the line number of the
 * source location. For an invalid source location, zero is returned.
 *
 * \param column [out] if non-NULL, will be set to the column number of the
 * source location. For an invalid source location, zero is returned.
 */
void clang_getPresumedLocation(CXSourceLocation location,
                                              CXString *filename,
                                              unsigned *line,
                                              unsigned *column);

/**
 * \brief Legacy API to retrieve the file, line, column, and offset represented
 * by the given source location.
 *
 * This interface has been replaced by the newer interface
 * #clang_getExpansionLocation(). See that interface's documentation for
 * details.
 */
void clang_getInstantiationLocation(CXSourceLocation location,
                                                   CXFile *file,
                                                   unsigned *line,
                                                   unsigned *column,
                                                   unsigned *offset);

/**
 * \brief Retrieve the file, line, column, and offset represented by
 * the given source location.
 *
 * If the location refers into a macro instantiation, return where the
 * location was originally spelled in the source file.
 *
 * \param location the location within a source file that will be decomposed
 * into its parts.
 *
 * \param file [out] if non-NULL, will be set to the file to which the given
 * source location points.
 *
 * \param line [out] if non-NULL, will be set to the line to which the given
 * source location points.
 *
 * \param column [out] if non-NULL, will be set to the column to which the given
 * source location points.
 *
 * \param offset [out] if non-NULL, will be set to the offset into the
 * buffer to which the given source location points.
 */
void clang_getSpellingLocation(CXSourceLocation location,
                                              CXFile *file,
                                              unsigned *line,
                                              unsigned *column,
                                              unsigned *offset);

/**
 * \brief Retrieve the file, line, column, and offset represented by
 * the given source location.
 *
 * If the location refers into a macro expansion, return where the macro was
 * expanded or where the macro argument was written, if the location points at
 * a macro argument.
 *
 * \param location the location within a source file that will be decomposed
 * into its parts.
 *
 * \param file [out] if non-NULL, will be set to the file to which the given
 * source location points.
 *
 * \param line [out] if non-NULL, will be set to the line to which the given
 * source location points.
 *
 * \param column [out] if non-NULL, will be set to the column to which the given
 * source location points.
 *
 * \param offset [out] if non-NULL, will be set to the offset into the
 * buffer to which the given source location points.
 */
void clang_getFileLocation(CXSourceLocation location,
                                          CXFile *file,
                                          unsigned *line,
                                          unsigned *column,
                                          unsigned *offset);

/**
 * \brief Retrieve a diagnostic associated with the given CXDiagnosticSet.
 *
 * \param Diags the CXDiagnosticSet to query.
 * \param Index the zero-based diagnostic number to retrieve.
 *
 * \returns the requested diagnostic. This diagnostic must be freed
 * via a call to \c clang_disposeDiagnostic().
 */
CXDiagnostic clang_getDiagnosticInSet(CXDiagnosticSet Diags,
                                                     unsigned Index);

/**
 * \brief Deserialize a set of diagnostics from a Clang diagnostics bitcode
 * file.
 *
 * \param file The name of the file to deserialize.
 * \param error A pointer to a enum value recording if there was a problem
 *        deserializing the diagnostics.
 * \param errorString A pointer to a CXString for recording the error string
 *        if the file was not successfully loaded.
 *
 * \returns A loaded CXDiagnosticSet if successful, and NULL otherwise.  These
 * diagnostics should be released using clang_disposeDiagnosticSet().
 */
CXDiagnosticSet clang_loadDiagnostics(const char *file,
                                                  enum CXLoadDiag_Error *error,
                                                  CXString *errorString);

/**
 * \brief Retrieve a diagnostic associated with the given translation unit.
 *
 * \param Unit the translation unit to query.
 * \param Index the zero-based diagnostic number to retrieve.
 *
 * \returns the requested diagnostic. This diagnostic must be freed
 * via a call to \c clang_disposeDiagnostic().
 */
CXDiagnostic clang_getDiagnostic(CXTranslationUnit Unit,
                                                unsigned Index);

/**
 * \brief Format the given diagnostic in a manner that is suitable for display.
 *
 * This routine will format the given diagnostic to a string, rendering
 * the diagnostic according to the various options given. The
 * \c clang_defaultDiagnosticDisplayOptions() function returns the set of
 * options that most closely mimics the behavior of the clang compiler.
 *
 * \param Diagnostic The diagnostic to print.
 *
 * \param Options A set of options that control the diagnostic display,
 * created by combining \c CXDiagnosticDisplayOptions values.
 *
 * \returns A new string containing for formatted diagnostic.
 */
CXString clang_formatDiagnostic(CXDiagnostic Diagnostic,
                                               unsigned Options);

/**
 * \brief Retrieve the set of display options most similar to the
 * default behavior of the clang compiler.
 *
 * \returns A set of display options suitable for use with \c
 * clang_formatDiagnostic().
 */
unsigned clang_defaultDiagnosticDisplayOptions(void);

/**
 * \brief Retrieve the name of the command-line option that enabled this
 * diagnostic.
 *
 * \param Diag The diagnostic to be queried.
 *
 * \param Disable If non-NULL, will be set to the option that disables this
 * diagnostic (if any).
 *
 * \returns A string that contains the command-line option used to enable this
 * warning, such as "-Wconversion" or "-pedantic".
 */
CXString clang_getDiagnosticOption(CXDiagnostic Diag,
                                                  CXString *Disable);

/**
 * \brief Retrieve the name of a particular diagnostic category.  This
 *  is now deprecated.  Use clang_getDiagnosticCategoryText()
 *  instead.
 *
 * \param Category A diagnostic category number, as returned by
 * \c clang_getDiagnosticCategory().
 *
 * \returns The name of the given diagnostic category.
 */
CXString clang_getDiagnosticCategoryName(unsigned Category);

/**
 * \brief Retrieve a source range associated with the diagnostic.
 *
 * A diagnostic's source ranges highlight important elements in the source
 * code. On the command line, Clang displays source ranges by
 * underlining them with '~' characters.
 *
 * \param Diagnostic the diagnostic whose range is being extracted.
 *
 * \param Range the zero-based index specifying which range to
 *
 * \returns the requested source range.
 */
CXSourceRange clang_getDiagnosticRange(CXDiagnostic Diagnostic,
                                                      unsigned Range);

/**
 * \brief Retrieve the replacement information for a given fix-it.
 *
 * Fix-its are described in terms of a source range whose contents
 * should be replaced by a string. This approach generalizes over
 * three kinds of operations: removal of source code (the range covers
 * the code to be removed and the replacement string is empty),
 * replacement of source code (the range covers the code to be
 * replaced and the replacement string provides the new code), and
 * insertion (both the start and end of the range point at the
 * insertion location, and the replacement string provides the text to
 * insert).
 *
 * \param Diagnostic The diagnostic whose fix-its are being queried.
 *
 * \param FixIt The zero-based index of the fix-it.
 *
 * \param ReplacementRange The source range whose contents will be
 * replaced with the returned replacement string. Note that source
 * ranges are half-open ranges [a, b), so the source code should be
 * replaced from a and up to (but not including) b.
 *
 * \returns A string containing text that should be replace the source
 * code indicated by the \c ReplacementRange.
 */
CXString clang_getDiagnosticFixIt(CXDiagnostic Diagnostic,
                                                 unsigned FixIt,
                                               CXSourceRange *ReplacementRange);

/**
 * \brief Get the original translation unit source file name.
 */
CXString
clang_getTranslationUnitSpelling(CXTranslationUnit CTUnit);

/**
 * \brief Return the CXTranslationUnit for a given source file and the provided
 * command line arguments one would pass to the compiler.
 *
 * Note: The 'source_filename' argument is optional.  If the caller provides a
 * NULL pointer, the name of the source file is expected to reside in the
 * specified command line arguments.
 *
 * Note: When encountered in 'clang_command_line_args', the following options
 * are ignored:
 *
 *   '-c'
 *   '-emit-ast'
 *   '-fsyntax-only'
 *   '-o \<output file>'  (both '-o' and '\<output file>' are ignored)
 *
 * \param CIdx The index object with which the translation unit will be
 * associated.
 *
 * \param source_filename The name of the source file to load, or NULL if the
 * source file is included in \p clang_command_line_args.
 *
 * \param num_clang_command_line_args The number of command-line arguments in
 * \p clang_command_line_args.
 *
 * \param clang_command_line_args The command-line arguments that would be
 * passed to the \c clang executable if it were being invoked out-of-process.
 * These command-line options will be parsed and will affect how the translation
 * unit is parsed. Note that the following options are ignored: '-c',
 * '-emit-ast', '-fsyntax-only' (which is the default), and '-o \<output file>'.
 *
 * \param num_unsaved_files the number of unsaved file entries in \p
 * unsaved_files.
 *
 * \param unsaved_files the files that have not yet been saved to disk
 * but may be required for code completion, including the contents of
 * those files.  The contents and name of these files (as specified by
 * CXUnsavedFile) are copied when necessary, so the client only needs to
 * guarantee their validity until the call to this function returns.
 */
CXTranslationUnit clang_createTranslationUnitFromSourceFile(
                                         CXIndex CIdx,
                                         const char *source_filename,
                                         int num_clang_command_line_args,
                                   const char * const *clang_command_line_args,
                                         unsigned num_unsaved_files,
                                         struct CXUnsavedFile *unsaved_files);

/**
 * \brief Create a translation unit from an AST file (-emit-ast).
 */
CXTranslationUnit clang_createTranslationUnit(CXIndex,
                                             const char *ast_filename);

/**
 * \brief Returns the set of flags that is suitable for parsing a translation
 * unit that is being edited.
 *
 * The set of flags returned provide options for \c clang_parseTranslationUnit()
 * to indicate that the translation unit is likely to be reparsed many times,
 * either explicitly (via \c clang_reparseTranslationUnit()) or implicitly
 * (e.g., by code completion (\c clang_codeCompletionAt())). The returned flag
 * set contains an unspecified set of optimizations (e.g., the precompiled
 * preamble) geared toward improving the performance of these routines. The
 * set of optimizations enabled may change from one version to the next.
 */
unsigned clang_defaultEditingTranslationUnitOptions(void);

/**
 * \brief Parse the given source file and the translation unit corresponding
 * to that file.
 *
 * This routine is the main entry point for the Clang C API, providing the
 * ability to parse a source file into a translation unit that can then be
 * queried by other functions in the API. This routine accepts a set of
 * command-line arguments so that the compilation can be configured in the same
 * way that the compiler is configured on the command line.
 *
 * \param CIdx The index object with which the translation unit will be
 * associated.
 *
 * \param source_filename The name of the source file to load, or NULL if the
 * source file is included in \p command_line_args.
 *
 * \param command_line_args The command-line arguments that would be
 * passed to the \c clang executable if it were being invoked out-of-process.
 * These command-line options will be parsed and will affect how the translation
 * unit is parsed. Note that the following options are ignored: '-c',
 * '-emit-ast', '-fsyntax-only' (which is the default), and '-o \<output file>'.
 *
 * \param num_command_line_args The number of command-line arguments in
 * \p command_line_args.
 *
 * \param unsaved_files the files that have not yet been saved to disk
 * but may be required for parsing, including the contents of
 * those files.  The contents and name of these files (as specified by
 * CXUnsavedFile) are copied when necessary, so the client only needs to
 * guarantee their validity until the call to this function returns.
 *
 * \param num_unsaved_files the number of unsaved file entries in \p
 * unsaved_files.
 *
 * \param options A bitmask of options that affects how the translation unit
 * is managed but not its compilation. This should be a bitwise OR of the
 * CXTranslationUnit_XXX flags.
 *
 * \returns A new translation unit describing the parsed code and containing
 * any diagnostics produced by the compiler. If there is a failure from which
 * the compiler cannot recover, returns NULL.
 */
CXTranslationUnit clang_parseTranslationUnit(CXIndex CIdx,
                                                    const char *source_filename,
                                         const char * const *command_line_args,
                                                      int num_command_line_args,
                                            struct CXUnsavedFile *unsaved_files,
                                                     unsigned num_unsaved_files,
                                                            unsigned options);

/**
 * \brief Saves a translation unit into a serialized representation of
 * that translation unit on disk.
 *
 * Any translation unit that was parsed without error can be saved
 * into a file. The translation unit can then be deserialized into a
 * new \c CXTranslationUnit with \c clang_createTranslationUnit() or,
 * if it is an incomplete translation unit that corresponds to a
 * header, used as a precompiled header when parsing other translation
 * units.
 *
 * \param TU The translation unit to save.
 *
 * \param FileName The file to which the translation unit will be saved.
 *
 * \param options A bitmask of options that affects how the translation unit
 * is saved. This should be a bitwise OR of the
 * CXSaveTranslationUnit_XXX flags.
 *
 * \returns A value that will match one of the enumerators of the CXSaveError
 * enumeration. Zero (CXSaveError_None) indicates that the translation unit was
 * saved successfully, while a non-zero value indicates that a problem occurred.
 */
int clang_saveTranslationUnit(CXTranslationUnit TU,
                                             const char *FileName,
                                             unsigned options);

/**
 * \brief Reparse the source files that produced this translation unit.
 *
 * This routine can be used to re-parse the source files that originally
 * created the given translation unit, for example because those source files
 * have changed (either on disk or as passed via \p unsaved_files). The
 * source code will be reparsed with the same command-line options as it
 * was originally parsed.
 *
 * Reparsing a translation unit invalidates all cursors and source locations
 * that refer into that translation unit. This makes reparsing a translation
 * unit semantically equivalent to destroying the translation unit and then
 * creating a new translation unit with the same command-line arguments.
 * However, it may be more efficient to reparse a translation
 * unit using this routine.
 *
 * \param TU The translation unit whose contents will be re-parsed. The
 * translation unit must originally have been built with
 * \c clang_createTranslationUnitFromSourceFile().
 *
 * \param num_unsaved_files The number of unsaved file entries in \p
 * unsaved_files.
 *
 * \param unsaved_files The files that have not yet been saved to disk
 * but may be required for parsing, including the contents of
 * those files.  The contents and name of these files (as specified by
 * CXUnsavedFile) are copied when necessary, so the client only needs to
 * guarantee their validity until the call to this function returns.
 *
 * \param options A bitset of options composed of the flags in CXReparse_Flags.
 * The function \c clang_defaultReparseOptions() produces a default set of
 * options recommended for most uses, based on the translation unit.
 *
 * \returns 0 if the sources could be reparsed. A non-zero value will be
 * returned if reparsing was impossible, such that the translation unit is
 * invalid. In such cases, the only valid call for \p TU is
 * \c clang_disposeTranslationUnit(TU).
 */
int clang_reparseTranslationUnit(CXTranslationUnit TU,
                                                unsigned num_unsaved_files,
                                          struct CXUnsavedFile *unsaved_files,
                                                unsigned options);

/**
  * \brief Returns the human-readable null-terminated C string that represents
  *  the name of the memory category.  This string should never be freed.
  */
const char *clang_getTUResourceUsageName(enum CXTUResourceUsageKind kind);

typedef struct CXTUResourceUsageEntry {
  /* \brief The memory usage category. */
  enum CXTUResourceUsageKind kind;
  /* \brief Amount of resources used.
      The units will depend on the resource kind. */
  unsigned long amount;
} CXTUResourceUsageEntry;

/**
  * \brief The memory usage of a CXTranslationUnit, broken into categories.
  */
typedef struct CXTUResourceUsage {
  /* \brief Private data member, used for queries. */
  void *data;

  /* \brief The number of entries in the 'entries' array. */
  unsigned numEntries;

  /* \brief An array of key-value pairs, representing the breakdown of memory
            usage. */
  CXTUResourceUsageEntry *entries;

} CXTUResourceUsage;

/**
 * \brief A cursor representing some element in the abstract syntax tree for
 * a translation unit.
 *
 * The cursor abstraction unifies the different kinds of entities in a
 * program--declaration, statements, expressions, references to declarations,
 * etc.--under a single "cursor" abstraction with a common set of operations.
 * Common operation for a cursor include: getting the physical location in
 * a source file where the cursor points, getting the name associated with a
 * cursor, and retrieving cursors for any child nodes of a particular cursor.
 *
 * Cursors can be produced in two specific ways.
 * clang_getTranslationUnitCursor() produces a cursor for a translation unit,
 * from which one can use clang_visitChildren() to explore the rest of the
 * translation unit. clang_getCursor() maps from a physical source location
 * to the entity that resides at that location, allowing one to map from the
 * source code into the AST.
 */
typedef struct {
  enum CXCursorKind kind;
  int xdata;
  const void *data[3];
} CXCursor;

/**
 * \brief A comment AST node.
 */
typedef struct {
  const void *ASTNode;
  CXTranslationUnit TranslationUnit;
} CXComment;

/**
 * Describes the availability of a given entity on a particular platform, e.g.,
 * a particular class might only be available on Mac OS 10.7 or newer.
 */
typedef struct CXPlatformAvailability {
  /**
   * \brief A string that describes the platform for which this structure
   * provides availability information.
   *
   * Possible values are "ios" or "macosx".
   */
  CXString Platform;
  /**
   * \brief The version number in which this entity was introduced.
   */
  CXVersion Introduced;
  /**
   * \brief The version number in which this entity was deprecated (but is
   * still available).
   */
  CXVersion Deprecated;
  /**
   * \brief The version number in which this entity was obsoleted, and therefore
   * is no longer available.
   */
  CXVersion Obsoleted;
  /**
   * \brief Whether the entity is unconditionally unavailable on this platform.
   */
  int Unavailable;
  /**
   * \brief An optional message to provide to a user of this API, e.g., to
   * suggest replacement APIs.
   */
  CXString Message;
} CXPlatformAvailability;

/**
 * \brief Determine the availability of the entity that this cursor refers to
 * on any platforms for which availability information is known.
 *
 * \param cursor The cursor to query.
 *
 * \param always_deprecated If non-NULL, will be set to indicate whether the
 * entity is deprecated on all platforms.
 *
 * \param deprecated_message If non-NULL, will be set to the message text
 * provided along with the unconditional deprecation of this entity. The client
 * is responsible for deallocating this string.
 *
 * \param always_unavailable If non-NULL, will be set to indicate whether the
 * entity is unavailable on all platforms.
 *
 * \param unavailable_message If non-NULL, will be set to the message text
 * provided along with the unconditional unavailability of this entity. The
 * client is responsible for deallocating this string.
 *
 * \param availability If non-NULL, an array of CXPlatformAvailability instances
 * that will be populated with platform availability information, up to either
 * the number of platforms for which availability information is available (as
 * returned by this function) or \c availability_size, whichever is smaller.
 *
 * \param availability_size The number of elements available in the
 * \c availability array.
 *
 * \returns The number of platforms (N) for which availability information is
 * available (which is unrelated to \c availability_size).
 *
 * Note that the client is responsible for calling
 * \c clang_disposeCXPlatformAvailability to free each of the
 * platform-availability structures returned. There are
 * \c min(N, availability_size) such structures.
 */
int
clang_getCursorPlatformAvailability(CXCursor cursor,
                                    int *always_deprecated,
                                    CXString *deprecated_message,
                                    int *always_unavailable,
                                    CXString *unavailable_message,
                                    CXPlatformAvailability *availability,
                                    int availability_size);

/**
 * \brief Free the memory associated with a \c CXPlatformAvailability structure.
 */
void
clang_disposeCXPlatformAvailability(CXPlatformAvailability *availability);

/**
 * \brief Queries a CXCursorSet to see if it contains a specific CXCursor.
 *
 * \returns non-zero if the set contains the specified cursor.
*/
unsigned clang_CXCursorSet_contains(CXCursorSet cset,
                                                   CXCursor cursor);

/**
 * \brief Inserts a CXCursor into a CXCursorSet.
 *
 * \returns zero if the CXCursor was already in the set, and non-zero otherwise.
*/
unsigned clang_CXCursorSet_insert(CXCursorSet cset,
                                                 CXCursor cursor);

/**
 * \brief Determine the set of methods that are overridden by the given
 * method.
 *
 * In both Objective-C and C++, a method (aka virtual member function,
 * in C++) can override a virtual method in a base class. For
 * Objective-C, a method is said to override any method in the class's
 * base class, its protocols, or its categories' protocols, that has the same
 * selector and is of the same kind (class or instance).
 * If no such method exists, the search continues to the class's superclass,
 * its protocols, and its categories, and so on. A method from an Objective-C
 * implementation is considered to override the same methods as its
 * corresponding method in the interface.
 *
 * For C++, a virtual member function overrides any virtual member
 * function with the same signature that occurs in its base
 * classes. With multiple inheritance, a virtual member function can
 * override several virtual member functions coming from different
 * base classes.
 *
 * In all cases, this function determines the immediate overridden
 * method, rather than all of the overridden methods. For example, if
 * a method is originally declared in a class A, then overridden in B
 * (which in inherits from A) and also in C (which inherited from B),
 * then the only overridden method returned from this function when
 * invoked on C's method will be B's method. The client may then
 * invoke this function again, given the previously-found overridden
 * methods, to map out the complete method-override set.
 *
 * \param cursor A cursor representing an Objective-C or C++
 * method. This routine will compute the set of methods that this
 * method overrides.
 *
 * \param overridden A pointer whose pointee will be replaced with a
 * pointer to an array of cursors, representing the set of overridden
 * methods. If there are no overridden methods, the pointee will be
 * set to NULL. The pointee must be freed via a call to
 * \c clang_disposeOverriddenCursors().
 *
 * \param num_overridden A pointer to the number of overridden
 * functions, will be set to the number of overridden functions in the
 * array pointed to by \p overridden.
 */
void clang_getOverriddenCursors(CXCursor cursor,
                                               CXCursor **overridden,
                                               unsigned *num_overridden);

/**
 * \brief Free the set of overridden cursors returned by \c
 * clang_getOverriddenCursors().
 */
void clang_disposeOverriddenCursors(CXCursor *overridden);

/**
 * \brief Map a source location to the cursor that describes the entity at that
 * location in the source code.
 *
 * clang_getCursor() maps an arbitrary source location within a translation
 * unit down to the most specific cursor that describes the entity at that
 * location. For example, given an expression \c x + y, invoking
 * clang_getCursor() with a source location pointing to "x" will return the
 * cursor for "x"; similarly for "y". If the cursor points anywhere between
 * "x" or "y" (e.g., on the + or the whitespace around it), clang_getCursor()
 * will return a cursor referring to the "+" expression.
 *
 * \returns a cursor representing the entity at the given source location, or
 * a NULL cursor if no such entity can be found.
 */
CXCursor clang_getCursor(CXTranslationUnit, CXSourceLocation);

/**
 * \brief The type of an element in the abstract syntax tree.
 *
 */
typedef struct {
  enum CXTypeKind kind;
  void *data[2];
} CXType;

/**
 * \brief Retrieve the integer value of an enum constant declaration as a signed
 *  long long.
 *
 * If the cursor does not reference an enum constant declaration, LLONG_MIN is returned.
 * Since this is also potentially a valid constant value, the kind of the cursor
 * must be verified before calling this function.
 */
long long clang_getEnumConstantDeclValue(CXCursor C);

/**
 * \brief Retrieve the argument cursor of a function or method.
 *
 * The argument cursor can be determined for calls as well as for declarations
 * of functions or methods. For other cursors and for invalid indices, an
 * invalid cursor is returned.
 */
CXCursor clang_Cursor_getArgument(CXCursor C, unsigned i);

/**
 * \brief Retrieve the type of an argument of a function type.
 *
 * If a non-function type is passed in or the function does not have enough
 * parameters, an invalid type is returned.
 */
CXType clang_getArgType(CXType T, unsigned i);

/**
 * \brief Return the number of elements of an array or vector type.
 *
 * If a type is passed in that is not an array or vector type,
 * -1 is returned.
 */
long long clang_getNumElements(CXType T);

/**
 * \brief Return the array size of a constant array.
 *
 * If a non-array type is passed in, -1 is returned.
 */
long long clang_getArraySize(CXType T);

/**
 * \brief Return the alignment of a type in bytes as per C++[expr.alignof]
 *   standard.
 *
 * If the type declaration is invalid, CXTypeLayoutError_Invalid is returned.
 * If the type declaration is an incomplete type, CXTypeLayoutError_Incomplete
 *   is returned.
 * If the type declaration is a dependent type, CXTypeLayoutError_Dependent is
 *   returned.
 * If the type declaration is not a constant size type,
 *   CXTypeLayoutError_NotConstantSize is returned.
 */
long long clang_Type_getAlignOf(CXType T);

/**
 * \brief Return the size of a type in bytes as per C++[expr.sizeof] standard.
 *
 * If the type declaration is invalid, CXTypeLayoutError_Invalid is returned.
 * If the type declaration is an incomplete type, CXTypeLayoutError_Incomplete
 *   is returned.
 * If the type declaration is a dependent type, CXTypeLayoutError_Dependent is
 *   returned.
 */
long long clang_Type_getSizeOf(CXType T);

/**
 * \brief Return the offset of a field named S in a record of type T in bits
 *   as it would be returned by __offsetof__ as per C++11[18.2p4]
 *
 * If the cursor is not a record field declaration, CXTypeLayoutError_Invalid
 *   is returned.
 * If the field's type declaration is an incomplete type,
 *   CXTypeLayoutError_Incomplete is returned.
 * If the field's type declaration is a dependent type,
 *   CXTypeLayoutError_Dependent is returned.
 * If the field's name S is not found,
 *   CXTypeLayoutError_InvalidFieldName is returned.
 */
long long clang_Type_getOffsetOf(CXType T, const char *S);

/**
 * \brief Retrieve a cursor for one of the overloaded declarations referenced
 * by a \c CXCursor_OverloadedDeclRef cursor.
 *
 * \param cursor The cursor whose overloaded declarations are being queried.
 *
 * \param index The zero-based index into the set of overloaded declarations in
 * the cursor.
 *
 * \returns A cursor representing the declaration referenced by the given
 * \c cursor at the specified \c index. If the cursor does not have an
 * associated set of overloaded declarations, or if the index is out of bounds,
 * returns \c clang_getNullCursor();
 */
CXCursor clang_getOverloadedDecl(CXCursor cursor, unsigned index);

/**
 * \brief Visitor invoked for each cursor found by a traversal.
 *
 * This visitor function will be invoked for each cursor found by
 * clang_visitCursorChildren(). Its first argument is the cursor being
 * visited, its second argument is the parent visitor for that cursor,
 * and its third argument is the client data provided to
 * clang_visitCursorChildren().
 *
 * The visitor should return one of the \c CXChildVisitResult values
 * to direct clang_visitCursorChildren().
 */
typedef enum CXChildVisitResult (*CXCursorVisitor)(CXCursor cursor,
                                                   CXCursor parent,
                                                   CXClientData client_data);

/**
 * \brief Visit the children of a particular cursor.
 *
 * This function visits all the direct children of the given cursor,
 * invoking the given \p visitor function with the cursors of each
 * visited child. The traversal may be recursive, if the visitor returns
 * \c CXChildVisit_Recurse. The traversal may also be ended prematurely, if
 * the visitor returns \c CXChildVisit_Break.
 *
 * \param parent the cursor whose child may be visited. All kinds of
 * cursors can be visited, including invalid cursors (which, by
 * definition, have no children).
 *
 * \param visitor the visitor function that will be invoked for each
 * child of \p parent.
 *
 * \param client_data pointer data supplied by the client, which will
 * be passed to the visitor each time it is invoked.
 *
 * \returns a non-zero value if the traversal was terminated
 * prematurely by the visitor returning \c CXChildVisit_Break.
 */
unsigned clang_visitChildren(CXCursor parent,
                                            CXCursorVisitor visitor,
                                            CXClientData client_data);
#ifdef __has_feature
#  if __has_feature(blocks)
/**
 * \brief Visitor invoked for each cursor found by a traversal.
 *
 * This visitor block will be invoked for each cursor found by
 * clang_visitChildrenWithBlock(). Its first argument is the cursor being
 * visited, its second argument is the parent visitor for that cursor.
 *
 * The visitor should return one of the \c CXChildVisitResult values
 * to direct clang_visitChildrenWithBlock().
 */
typedef enum CXChildVisitResult
     (^CXCursorVisitorBlock)(CXCursor cursor, CXCursor parent);

/**
 * Visits the children of a cursor using the specified block.  Behaves
 * identically to clang_visitChildren() in all other respects.
 */
unsigned clang_visitChildrenWithBlock(CXCursor parent,
                                      CXCursorVisitorBlock block);
#  endif
#endif

/**
 * \brief Construct a USR for a specified Objective-C class.
 */
CXString clang_constructUSR_ObjCClass(const char *class_name);

/**
 * \brief Construct a USR for a specified Objective-C category.
 */
CXString
  clang_constructUSR_ObjCCategory(const char *class_name,
                                 const char *category_name);

/**
 * \brief Construct a USR for a specified Objective-C protocol.
 */
CXString
  clang_constructUSR_ObjCProtocol(const char *protocol_name);


/**
 * \brief Construct a USR for a specified Objective-C instance variable and
 *   the USR for its containing class.
 */
CXString clang_constructUSR_ObjCIvar(const char *name,
                                                    CXString classUSR);

/**
 * \brief Construct a USR for a specified Objective-C method and
 *   the USR for its containing class.
 */
CXString clang_constructUSR_ObjCMethod(const char *name,
                                                      unsigned isInstanceMethod,
                                                      CXString classUSR);

/**
 * \brief Construct a USR for a specified Objective-C property and the USR
 *  for its containing class.
 */
CXString clang_constructUSR_ObjCProperty(const char *property,
                                                        CXString classUSR);

/**
 * \brief Retrieve a range for a piece that forms the cursors spelling name.
 * Most of the times there is only one range for the complete spelling but for
 * objc methods and objc message expressions, there are multiple pieces for each
 * selector identifier.
 *
 * \param pieceIndex the index of the spelling name piece. If this is greater
 * than the actual number of pieces, it will return a NULL (invalid) range.
 *
 * \param options Reserved.
 */
CXSourceRange clang_Cursor_getSpellingNameRange(CXCursor,
                                                          unsigned pieceIndex,
                                                          unsigned options);

/**
 * \brief Given a cursor that represents a property declaration, return the
 * associated property attributes. The bits are formed from
 * \c CXObjCPropertyAttrKind.
 *
 * \param reserved Reserved for future use, pass 0.
 */
unsigned clang_Cursor_getObjCPropertyAttributes(CXCursor C, unsigned reserved);

/**
 * \param Module a module object.
 *
 * \returns the number of top level headers associated with this module.
 */
unsigned clang_Module_getNumTopLevelHeaders(CXTranslationUnit,
                                                           CXModule Module);

/**
 * \param Module a module object.
 *
 * \param Index top level header index (zero-based).
 *
 * \returns the specified top level header associated with the module.
 */
CXFile clang_Module_getTopLevelHeader(CXTranslationUnit,
                                      CXModule Module, unsigned Index);

/**
 * \param Comment AST node of any kind.
 *
 * \param ChildIdx child index (zero-based).
 *
 * \returns the specified child of the AST node.
 */
CXComment clang_Comment_getChild(CXComment Comment, unsigned ChildIdx);

/**
 * \param Comment a \c CXComment_InlineCommand AST node.
 *
 * \param ArgIdx argument index (zero-based).
 *
 * \returns text of the specified argument.
 */
CXString clang_InlineCommandComment_getArgText(CXComment Comment,
                                               unsigned ArgIdx);

/**
 * \param Comment a \c CXComment_HTMLStartTag AST node.
 *
 * \param AttrIdx attribute index (zero-based).
 *
 * \returns name of the specified attribute.
 */
CXString clang_HTMLStartTag_getAttrName(CXComment Comment, unsigned AttrIdx);

/**
 * \param Comment a \c CXComment_HTMLStartTag AST node.
 *
 * \param AttrIdx attribute index (zero-based).
 *
 * \returns value of the specified attribute.
 */
CXString clang_HTMLStartTag_getAttrValue(CXComment Comment, unsigned AttrIdx);

/**
 * \param Comment a \c CXComment_BlockCommand AST node.
 *
 * \param ArgIdx argument index (zero-based).
 *
 * \returns text of the specified word-like argument.
 */
CXString clang_BlockCommandComment_getArgText(CXComment Comment,
                                              unsigned ArgIdx);

/**
 * \param Comment a \c CXComment_TParamCommand AST node.
 *
 * \returns zero-based parameter index in the template parameter list at a
 * given nesting depth.
 *
 * For example,
 * \verbatim
 *     template<typename C, template<typename T> class TT>
 *     void test(TT<int> aaa);
 * \endverbatim
 * for C and TT nesting depth is 0, so we can ask for index at depth 0:
 * at depth 0 C's index is 0, TT's index is 1.
 *
 * For T nesting depth is 1, so we can ask for index at depth 0 and 1:
 * at depth 0 T's index is 1 (same as TT's),
 * at depth 1 T's index is 0.
 */
unsigned clang_TParamCommandComment_getIndex(CXComment Comment, unsigned Depth);

/**
 * \brief Given a cursor that references something else, return the source range
 * covering that reference.
 *
 * \param C A cursor pointing to a member reference, a declaration reference, or
 * an operator call.
 * \param NameFlags A bitset with three independent flags:
 * CXNameRange_WantQualifier, CXNameRange_WantTemplateArgs, and
 * CXNameRange_WantSinglePiece.
 * \param PieceIndex For contiguous names or when passing the flag
 * CXNameRange_WantSinglePiece, only one piece with index 0 is
 * available. When the CXNameRange_WantSinglePiece flag is not passed for a
 * non-contiguous names, this index can be used to retrieve the individual
 * pieces of the name. See also CXNameRange_WantSinglePiece.
 *
 * \returns The piece of the name pointed to by the given cursor. If there is no
 * name, or if the PieceIndex is out-of-range, a null-cursor will be returned.
 */
CXSourceRange clang_getCursorReferenceNameRange(CXCursor C,
                                                unsigned NameFlags,
                                                unsigned PieceIndex);

/**
 * \brief Describes a single preprocessing token.
 */
typedef struct {
  unsigned int_data[4];
  void *ptr_data;
} CXToken;

/**
 * \brief Determine the spelling of the given token.
 *
 * The spelling of a token is the textual representation of that token, e.g.,
 * the text of an identifier or keyword.
 */
CXString clang_getTokenSpelling(CXTranslationUnit, CXToken);

/**
 * \brief Retrieve the source location of the given token.
 */
CXSourceLocation clang_getTokenLocation(CXTranslationUnit,
                                                       CXToken);

/**
 * \brief Retrieve a source range that covers the given token.
 */
CXSourceRange clang_getTokenExtent(CXTranslationUnit, CXToken);

/**
 * \brief Tokenize the source code described by the given range into raw
 * lexical tokens.
 *
 * \param TU the translation unit whose text is being tokenized.
 *
 * \param Range the source range in which text should be tokenized. All of the
 * tokens produced by tokenization will fall within this source range,
 *
 * \param Tokens this pointer will be set to point to the array of tokens
 * that occur within the given source range. The returned pointer must be
 * freed with clang_disposeTokens() before the translation unit is destroyed.
 *
 * \param NumTokens will be set to the number of tokens in the \c *Tokens
 * array.
 *
 */
void clang_tokenize(CXTranslationUnit TU, CXSourceRange Range,
                                   CXToken **Tokens, unsigned *NumTokens);

/**
 * \brief Annotate the given set of tokens by providing cursors for each token
 * that can be mapped to a specific entity within the abstract syntax tree.
 *
 * This token-annotation routine is equivalent to invoking
 * clang_getCursor() for the source locations of each of the
 * tokens. The cursors provided are filtered, so that only those
 * cursors that have a direct correspondence to the token are
 * accepted. For example, given a function call \c f(x),
 * clang_getCursor() would provide the following cursors:
 *
 *   * when the cursor is over the 'f', a DeclRefExpr cursor referring to 'f'.
 *   * when the cursor is over the '(' or the ')', a CallExpr referring to 'f'.
 *   * when the cursor is over the 'x', a DeclRefExpr cursor referring to 'x'.
 *
 * Only the first and last of these cursors will occur within the
 * annotate, since the tokens "f" and "x' directly refer to a function
 * and a variable, respectively, but the parentheses are just a small
 * part of the full syntax of the function call expression, which is
 * not provided as an annotation.
 *
 * \param TU the translation unit that owns the given tokens.
 *
 * \param Tokens the set of tokens to annotate.
 *
 * \param NumTokens the number of tokens in \p Tokens.
 *
 * \param Cursors an array of \p NumTokens cursors, whose contents will be
 * replaced with the cursors corresponding to each token.
 */
void clang_annotateTokens(CXTranslationUnit TU,
                                         CXToken *Tokens, unsigned NumTokens,
                                         CXCursor *Cursors);

/**
 * \brief Free the given set of tokens.
 */
void clang_disposeTokens(CXTranslationUnit TU,
                                        CXToken *Tokens, unsigned NumTokens);

/* for debug/testing */
void clang_getDefinitionSpellingAndExtent(CXCursor,
                                          const char **startBuf,
                                          const char **endBuf,
                                          unsigned *startLine,
                                          unsigned *startColumn,
                                          unsigned *endLine,
                                          unsigned *endColumn);
void clang_enableStackTraces(void);
void clang_executeOnThread(void (*fn)(void*), void *user_data,
                                          unsigned stack_size);

/**
 * \brief A single result of code completion.
 */
typedef struct {
  /**
   * \brief The kind of entity that this completion refers to.
   *
   * The cursor kind will be a macro, keyword, or a declaration (one of the
   * *Decl cursor kinds), describing the entity that the completion is
   * referring to.
   *
   * \todo In the future, we would like to provide a full cursor, to allow
   * the client to extract additional information from declaration.
   */
  enum CXCursorKind CursorKind;

  /**
   * \brief The code-completion string that describes how to insert this
   * code-completion result into the editing buffer.
   */
  CXCompletionString CompletionString;
} CXCompletionResult;

/**
 * \brief Determine the kind of a particular chunk within a completion string.
 *
 * \param completion_string the completion string to query.
 *
 * \param chunk_number the 0-based index of the chunk in the completion string.
 *
 * \returns the kind of the chunk at the index \c chunk_number.
 */
enum CXCompletionChunkKind
clang_getCompletionChunkKind(CXCompletionString completion_string,
                             unsigned chunk_number);

/**
 * \brief Retrieve the text associated with a particular chunk within a
 * completion string.
 *
 * \param completion_string the completion string to query.
 *
 * \param chunk_number the 0-based index of the chunk in the completion string.
 *
 * \returns the text associated with the chunk at index \c chunk_number.
 */
CXString
clang_getCompletionChunkText(CXCompletionString completion_string,
                             unsigned chunk_number);

/**
 * \brief Retrieve the completion string associated with a particular chunk
 * within a completion string.
 *
 * \param completion_string the completion string to query.
 *
 * \param chunk_number the 0-based index of the chunk in the completion string.
 *
 * \returns the completion string associated with the chunk at index
 * \c chunk_number.
 */
CXCompletionString
clang_getCompletionChunkCompletionString(CXCompletionString completion_string,
                                         unsigned chunk_number);

/**
 * \brief Retrieve the annotation associated with the given completion string.
 *
 * \param completion_string the completion string to query.
 *
 * \param annotation_number the 0-based index of the annotation of the
 * completion string.
 *
 * \returns annotation string associated with the completion at index
 * \c annotation_number, or a NULL string if that annotation is not available.
 */
CXString
clang_getCompletionAnnotation(CXCompletionString completion_string,
                              unsigned annotation_number);

/**
 * \brief Retrieve the parent context of the given completion string.
 *
 * The parent context of a completion string is the semantic parent of
 * the declaration (if any) that the code completion represents. For example,
 * a code completion for an Objective-C method would have the method's class
 * or protocol as its context.
 *
 * \param completion_string The code completion string whose parent is
 * being queried.
 *
 * \param kind DEPRECATED: always set to CXCursor_NotImplemented if non-NULL.
 *
 * \returns The name of the completion parent, e.g., "NSObject" if
 * the completion string represents a method in the NSObject class.
 */
CXString
clang_getCompletionParent(CXCompletionString completion_string,
                          enum CXCursorKind *kind);

/**
 * \brief Contains the results of code-completion.
 *
 * This data structure contains the results of code completion, as
 * produced by \c clang_codeCompleteAt(). Its contents must be freed by
 * \c clang_disposeCodeCompleteResults.
 */
typedef struct {
  /**
   * \brief The code-completion results.
   */
  CXCompletionResult *Results;

  /**
   * \brief The number of code-completion results stored in the
   * \c Results array.
   */
  unsigned NumResults;
} CXCodeCompleteResults;

/**
 * \brief Returns a default set of code-completion options that can be
 * passed to\c clang_codeCompleteAt().
 */
unsigned clang_defaultCodeCompleteOptions(void);

/**
 * \brief Perform code completion at a given location in a translation unit.
 *
 * This function performs code completion at a particular file, line, and
 * column within source code, providing results that suggest potential
 * code snippets based on the context of the completion. The basic model
 * for code completion is that Clang will parse a complete source file,
 * performing syntax checking up to the location where code-completion has
 * been requested. At that point, a special code-completion token is passed
 * to the parser, which recognizes this token and determines, based on the
 * current location in the C/Objective-C/C++ grammar and the state of
 * semantic analysis, what completions to provide. These completions are
 * returned via a new \c CXCodeCompleteResults structure.
 *
 * Code completion itself is meant to be triggered by the client when the
 * user types punctuation characters or whitespace, at which point the
 * code-completion location will coincide with the cursor. For example, if \c p
 * is a pointer, code-completion might be triggered after the "-" and then
 * after the ">" in \c p->. When the code-completion location is afer the ">",
 * the completion results will provide, e.g., the members of the struct that
 * "p" points to. The client is responsible for placing the cursor at the
 * beginning of the token currently being typed, then filtering the results
 * based on the contents of the token. For example, when code-completing for
 * the expression \c p->get, the client should provide the location just after
 * the ">" (e.g., pointing at the "g") to this code-completion hook. Then, the
 * client can filter the results based on the current token text ("get"), only
 * showing those results that start with "get". The intent of this interface
 * is to separate the relatively high-latency acquisition of code-completion
 * results from the filtering of results on a per-character basis, which must
 * have a lower latency.
 *
 * \param TU The translation unit in which code-completion should
 * occur. The source files for this translation unit need not be
 * completely up-to-date (and the contents of those source files may
 * be overridden via \p unsaved_files). Cursors referring into the
 * translation unit may be invalidated by this invocation.
 *
 * \param complete_filename The name of the source file where code
 * completion should be performed. This filename may be any file
 * included in the translation unit.
 *
 * \param complete_line The line at which code-completion should occur.
 *
 * \param complete_column The column at which code-completion should occur.
 * Note that the column should point just after the syntactic construct that
 * initiated code completion, and not in the middle of a lexical token.
 *
 * \param unsaved_files the Tiles that have not yet been saved to disk
 * but may be required for parsing or code completion, including the
 * contents of those files.  The contents and name of these files (as
 * specified by CXUnsavedFile) are copied when necessary, so the
 * client only needs to guarantee their validity until the call to
 * this function returns.
 *
 * \param num_unsaved_files The number of unsaved file entries in \p
 * unsaved_files.
 *
 * \param options Extra options that control the behavior of code
 * completion, expressed as a bitwise OR of the enumerators of the
 * CXCodeComplete_Flags enumeration. The
 * \c clang_defaultCodeCompleteOptions() function returns a default set
 * of code-completion options.
 *
 * \returns If successful, a new \c CXCodeCompleteResults structure
 * containing code-completion results, which should eventually be
 * freed with \c clang_disposeCodeCompleteResults(). If code
 * completion fails, returns NULL.
 */
CXCodeCompleteResults *clang_codeCompleteAt(CXTranslationUnit TU,
                                            const char *complete_filename,
                                            unsigned complete_line,
                                            unsigned complete_column,
                                            struct CXUnsavedFile *unsaved_files,
                                            unsigned num_unsaved_files,
                                            unsigned options);

/**
 * \brief Sort the code-completion results in case-insensitive alphabetical
 * order.
 *
 * \param Results The set of results to sort.
 * \param NumResults The number of results in \p Results.
 */
void clang_sortCodeCompletionResults(CXCompletionResult *Results,
                                     unsigned NumResults);

/**
 * \brief Free the given set of code-completion results.
 */
void clang_disposeCodeCompleteResults(CXCodeCompleteResults *Results);

/**
 * \brief Determine the number of diagnostics produced prior to the
 * location where code completion was performed.
 */
unsigned clang_codeCompleteGetNumDiagnostics(CXCodeCompleteResults *Results);

/**
 * \brief Retrieve a diagnostic associated with the given code completion.
 *
 * \param Results the code completion results to query.
 * \param Index the zero-based diagnostic number to retrieve.
 *
 * \returns the requested diagnostic. This diagnostic must be freed
 * via a call to \c clang_disposeDiagnostic().
 */
CXDiagnostic clang_codeCompleteGetDiagnostic(CXCodeCompleteResults *Results,
                                             unsigned Index);

/**
 * \brief Determines what compeltions are appropriate for the context
 * the given code completion.
 *
 * \param Results the code completion results to query
 *
 * \returns the kinds of completions that are appropriate for use
 * along with the given code completion results.
 */
unsigned long long clang_codeCompleteGetContexts(
                                                CXCodeCompleteResults *Results);

/**
 * \brief Returns the cursor kind for the container for the current code
 * completion context. The container is only guaranteed to be set for
 * contexts where a container exists (i.e. member accesses or Objective-C
 * message sends); if there is not a container, this function will return
 * CXCursor_InvalidCode.
 *
 * \param Results the code completion results to query
 *
 * \param IsIncomplete on return, this value will be false if Clang has complete
 * information about the container. If Clang does not have complete
 * information, this value will be true.
 *
 * \returns the container kind, or CXCursor_InvalidCode if there is not a
 * container
 */
enum CXCursorKind clang_codeCompleteGetContainerKind(
                                                 CXCodeCompleteResults *Results,
                                                     unsigned *IsIncomplete);

/**
 * \brief Returns the USR for the container for the current code completion
 * context. If there is not a container for the current context, this
 * function will return the empty string.
 *
 * \param Results the code completion results to query
 *
 * \returns the USR for the container
 */
CXString clang_codeCompleteGetContainerUSR(CXCodeCompleteResults *Results);


/**
 * \brief Returns the currently-entered selector for an Objective-C message
 * send, formatted like "initWithFoo:bar:". Only guaranteed to return a
 * non-empty string for CXCompletionContext_ObjCInstanceMessage and
 * CXCompletionContext_ObjCClassMessage.
 *
 * \param Results the code completion results to query
 *
 * \returns the selector (or partial selector) that has been entered thus far
 * for an Objective-C message send.
 */
CXString clang_codeCompleteGetObjCSelector(CXCodeCompleteResults *Results);

/**
 * \brief Return a version string, suitable for showing to a user, but not
 *        intended to be parsed (the format is not guaranteed to be stable).
 */
CXString clang_getClangVersion(void);


/**
 * \brief Enable/disable crash recovery.
 *
 * \param isEnabled Flag to indicate if crash recovery is enabled.  A non-zero
 *        value enables crash recovery, while 0 disables it.
 */
void clang_toggleCrashRecovery(unsigned isEnabled);

 /**
  * \brief Visitor invoked for each file in a translation unit
  *        (used with clang_getInclusions()).
  *
  * This visitor function will be invoked by clang_getInclusions() for each
  * file included (either at the top-level or by \#include directives) within
  * a translation unit.  The first argument is the file being included, and
  * the second and third arguments provide the inclusion stack.  The
  * array is sorted in order of immediate inclusion.  For example,
  * the first element refers to the location that included 'included_file'.
  */
typedef void (*CXInclusionVisitor)(CXFile included_file,
                                   CXSourceLocation* inclusion_stack,
                                   unsigned include_len,
                                   CXClientData client_data);

/**
 * \brief Visit the set of preprocessor inclusions in a translation unit.
 *   The visitor function is called with the provided data for every included
 *   file.  This does not include headers included by the PCH file (unless one
 *   is inspecting the inclusions in the PCH file itself).
 */
void clang_getInclusions(CXTranslationUnit tu,
                                        CXInclusionVisitor visitor,
                                        CXClientData client_data);

/**
 * \brief Retrieve a remapping.
 *
 * \param path the path that contains metadata about remappings.
 *
 * \returns the requested remapping. This remapping must be freed
 * via a call to \c clang_remap_dispose(). Can return NULL if an error occurred.
 */
CXRemapping clang_getRemappings(const char *path);

/**
 * \brief Retrieve a remapping.
 *
 * \param filePaths pointer to an array of file paths containing remapping info.
 *
 * \param numFiles number of file paths.
 *
 * \returns the requested remapping. This remapping must be freed
 * via a call to \c clang_remap_dispose(). Can return NULL if an error occurred.
 */
CXRemapping clang_getRemappingsFromFileList(const char **filePaths,
                                            unsigned numFiles);

/**
 * \brief Get the original and the associated filename from the remapping.
 *
 * \param original If non-NULL, will be set to the original filename.
 *
 * \param transformed If non-NULL, will be set to the filename that the original
 * is associated with.
 */
void clang_remap_getFilenames(CXRemapping, unsigned index,
                                     CXString *original, CXString *transformed);

typedef struct {
  void *context;
  enum CXVisitorResult (*visit)(void *context, CXCursor, CXSourceRange);
} CXCursorAndRangeVisitor;

typedef enum {
  /**
   * \brief Function returned successfully.
   */
  CXResult_Success = 0,
  /**
   * \brief One of the parameters was invalid for the function.
   */
  CXResult_Invalid = 1,
  /**
   * \brief The function was terminated by a callback (e.g. it returned
   * CXVisit_Break)
   */
  CXResult_VisitBreak = 2

} CXResult;

/**
 * \brief Find references of a declaration in a specific file.
 *
 * \param cursor pointing to a declaration or a reference of one.
 *
 * \param file to search for references.
 *
 * \param visitor callback that will receive pairs of CXCursor/CXSourceRange for
 * each reference found.
 * The CXSourceRange will point inside the file; if the reference is inside
 * a macro (and not a macro argument) the CXSourceRange will be invalid.
 *
 * \returns one of the CXResult enumerators.
 */
CXResult clang_findReferencesInFile(CXCursor cursor, CXFile file,
                                               CXCursorAndRangeVisitor visitor);

/**
 * \brief Find #import/#include directives in a specific file.
 *
 * \param TU translation unit containing the file to query.
 *
 * \param file to search for #import/#include directives.
 *
 * \param visitor callback that will receive pairs of CXCursor/CXSourceRange for
 * each directive found.
 *
 * \returns one of the CXResult enumerators.
 */
CXResult clang_findIncludesInFile(CXTranslationUnit TU,
                                                 CXFile file,
                                              CXCursorAndRangeVisitor visitor);

#ifdef __has_feature
#  if __has_feature(blocks)

typedef enum CXVisitorResult
    (^CXCursorAndRangeVisitorBlock)(CXCursor, CXSourceRange);

CXResult clang_findReferencesInFileWithBlock(CXCursor, CXFile,
                                             CXCursorAndRangeVisitorBlock);

CXResult clang_findIncludesInFileWithBlock(CXTranslationUnit, CXFile,
                                           CXCursorAndRangeVisitorBlock);

#  endif
#endif

/**
 * \brief Source location passed to index callbacks.
 */
typedef struct {
  void *ptr_data[2];
  unsigned int_data;
} CXIdxLoc;

/**
 * \brief Data for ppIncludedFile callback.
 */
typedef struct {
  /**
   * \brief Location of '#' in the \#include/\#import directive.
   */
  CXIdxLoc hashLoc;
  /**
   * \brief Filename as written in the \#include/\#import directive.
   */
  const char *filename;
  /**
   * \brief The actual file that the \#include/\#import directive resolved to.
   */
  CXFile file;
  int isImport;
  int isAngled;
  /**
   * \brief Non-zero if the directive was automatically turned into a module
   * import.
   */
  int isModuleImport;
} CXIdxIncludedFileInfo;

/**
 * \brief Data for IndexerCallbacks#importedASTFile.
 */
typedef struct {
  /**
   * \brief Top level AST file containing the imported PCH, module or submodule.
   */
  CXFile file;
  /**
   * \brief The imported module or NULL if the AST file is a PCH.
   */
  CXModule module;
  /**
   * \brief Location where the file is imported. Applicable only for modules.
   */
  CXIdxLoc loc;
  /**
   * \brief Non-zero if an inclusion directive was automatically turned into
   * a module import. Applicable only for modules.
   */
  int isImplicit;

} CXIdxImportedASTFileInfo;

typedef struct {
  CXIdxAttrKind kind;
  CXCursor cursor;
  CXIdxLoc loc;
} CXIdxAttrInfo;

typedef struct {
  CXIdxEntityKind kind;
  CXIdxEntityCXXTemplateKind templateKind;
  CXIdxEntityLanguage lang;
  const char *name;
  const char *USR;
  CXCursor cursor;
  const CXIdxAttrInfo *const *attributes;
  unsigned numAttributes;
} CXIdxEntityInfo;

typedef struct {
  CXCursor cursor;
} CXIdxContainerInfo;

typedef struct {
  const CXIdxAttrInfo *attrInfo;
  const CXIdxEntityInfo *objcClass;
  CXCursor classCursor;
  CXIdxLoc classLoc;
} CXIdxIBOutletCollectionAttrInfo;

typedef struct {
  const CXIdxEntityInfo *entityInfo;
  CXCursor cursor;
  CXIdxLoc loc;
  const CXIdxContainerInfo *semanticContainer;
  /**
   * \brief Generally same as #semanticContainer but can be different in
   * cases like out-of-line C++ member functions.
   */
  const CXIdxContainerInfo *lexicalContainer;
  int isRedeclaration;
  int isDefinition;
  int isContainer;
  const CXIdxContainerInfo *declAsContainer;
  /**
   * \brief Whether the declaration exists in code or was created implicitly
   * by the compiler, e.g. implicit objc methods for properties.
   */
  int isImplicit;
  const CXIdxAttrInfo *const *attributes;
  unsigned numAttributes;

  unsigned flags;

} CXIdxDeclInfo;

typedef struct {
  const CXIdxDeclInfo *declInfo;
  CXIdxObjCContainerKind kind;
} CXIdxObjCContainerDeclInfo;

typedef struct {
  const CXIdxEntityInfo *base;
  CXCursor cursor;
  CXIdxLoc loc;
} CXIdxBaseClassInfo;

typedef struct {
  const CXIdxEntityInfo *protocol;
  CXCursor cursor;
  CXIdxLoc loc;
} CXIdxObjCProtocolRefInfo;

typedef struct {
  const CXIdxObjCProtocolRefInfo *const *protocols;
  unsigned numProtocols;
} CXIdxObjCProtocolRefListInfo;

typedef struct {
  const CXIdxObjCContainerDeclInfo *containerInfo;
  const CXIdxBaseClassInfo *superInfo;
  const CXIdxObjCProtocolRefListInfo *protocols;
} CXIdxObjCInterfaceDeclInfo;

typedef struct {
  const CXIdxObjCContainerDeclInfo *containerInfo;
  const CXIdxEntityInfo *objcClass;
  CXCursor classCursor;
  CXIdxLoc classLoc;
  const CXIdxObjCProtocolRefListInfo *protocols;
} CXIdxObjCCategoryDeclInfo;

typedef struct {
  const CXIdxDeclInfo *declInfo;
  const CXIdxEntityInfo *getter;
  const CXIdxEntityInfo *setter;
} CXIdxObjCPropertyDeclInfo;

typedef struct {
  const CXIdxDeclInfo *declInfo;
  const CXIdxBaseClassInfo *const *bases;
  unsigned numBases;
} CXIdxCXXClassDeclInfo;

/**
 * \brief Data for IndexerCallbacks#indexEntityReference.
 */
typedef struct {
  CXIdxEntityRefKind kind;
  /**
   * \brief Reference cursor.
   */
  CXCursor cursor;
  CXIdxLoc loc;
  /**
   * \brief The entity that gets referenced.
   */
  const CXIdxEntityInfo *referencedEntity;
  /**
   * \brief Immediate "parent" of the reference. For example:
   *
   * \code
   * Foo *var;
   * \endcode
   *
   * The parent of reference of type 'Foo' is the variable 'var'.
   * For references inside statement bodies of functions/methods,
   * the parentEntity will be the function/method.
   */
  const CXIdxEntityInfo *parentEntity;
  /**
   * \brief Lexical container context of the reference.
   */
  const CXIdxContainerInfo *container;
} CXIdxEntityRefInfo;

/**
 * \brief A group of callbacks used by #clang_indexSourceFile and
 * #clang_indexTranslationUnit.
 */
typedef struct {
  /**
   * \brief Called periodically to check whether indexing should be aborted.
   * Should return 0 to continue, and non-zero to abort.
   */
  int (*abortQuery)(CXClientData client_data, void *reserved);

  /**
   * \brief Called at the end of indexing; passes the complete diagnostic set.
   */
  void (*diagnostic)(CXClientData client_data,
                     CXDiagnosticSet, void *reserved);

  CXIdxClientFile (*enteredMainFile)(CXClientData client_data,
                                     CXFile mainFile, void *reserved);

  /**
   * \brief Called when a file gets \#included/\#imported.
   */
  CXIdxClientFile (*ppIncludedFile)(CXClientData client_data,
                                    const CXIdxIncludedFileInfo *);

  /**
   * \brief Called when a AST file (PCH or module) gets imported.
   *
   * AST files will not get indexed (there will not be callbacks to index all
   * the entities in an AST file). The recommended action is that, if the AST
   * file is not already indexed, to initiate a new indexing job specific to
   * the AST file.
   */
  CXIdxClientASTFile (*importedASTFile)(CXClientData client_data,
                                        const CXIdxImportedASTFileInfo *);

  /**
   * \brief Called at the beginning of indexing a translation unit.
   */
  CXIdxClientContainer (*startedTranslationUnit)(CXClientData client_data,
                                                 void *reserved);

  void (*indexDeclaration)(CXClientData client_data,
                           const CXIdxDeclInfo *);

  /**
   * \brief Called to index a reference of an entity.
   */
  void (*indexEntityReference)(CXClientData client_data,
                               const CXIdxEntityRefInfo *);

} IndexerCallbacks;

const CXIdxObjCContainerDeclInfo *
clang_index_getObjCContainerDeclInfo(const CXIdxDeclInfo *);

const CXIdxObjCInterfaceDeclInfo *
clang_index_getObjCInterfaceDeclInfo(const CXIdxDeclInfo *);

const CXIdxObjCCategoryDeclInfo *
clang_index_getObjCCategoryDeclInfo(const CXIdxDeclInfo *);

const CXIdxObjCProtocolRefListInfo *
clang_index_getObjCProtocolRefListInfo(const CXIdxDeclInfo *);

const CXIdxObjCPropertyDeclInfo *
clang_index_getObjCPropertyDeclInfo(const CXIdxDeclInfo *);

const CXIdxIBOutletCollectionAttrInfo *
clang_index_getIBOutletCollectionAttrInfo(const CXIdxAttrInfo *);

const CXIdxCXXClassDeclInfo *
clang_index_getCXXClassDeclInfo(const CXIdxDeclInfo *);

/**
 * \brief For retrieving a custom CXIdxClientContainer attached to a
 * container.
 */
CXIdxClientContainer
clang_index_getClientContainer(const CXIdxContainerInfo *);

/**
 * \brief For setting a custom CXIdxClientContainer attached to a
 * container.
 */
void
clang_index_setClientContainer(const CXIdxContainerInfo *,CXIdxClientContainer);

/**
 * \brief For retrieving a custom CXIdxClientEntity attached to an entity.
 */
CXIdxClientEntity
clang_index_getClientEntity(const CXIdxEntityInfo *);

/**
 * \brief For setting a custom CXIdxClientEntity attached to an entity.
 */
void
clang_index_setClientEntity(const CXIdxEntityInfo *, CXIdxClientEntity);

/**
 * \brief Index the given source file and the translation unit corresponding
 * to that file via callbacks implemented through #IndexerCallbacks.
 *
 * \param client_data pointer data supplied by the client, which will
 * be passed to the invoked callbacks.
 *
 * \param index_callbacks Pointer to indexing callbacks that the client
 * implements.
 *
 * \param index_callbacks_size Size of #IndexerCallbacks structure that gets
 * passed in index_callbacks.
 *
 * \param index_options A bitmask of options that affects how indexing is
 * performed. This should be a bitwise OR of the CXIndexOpt_XXX flags.
 *
 * \param out_TU [out] pointer to store a CXTranslationUnit that can be reused
 * after indexing is finished. Set to NULL if you do not require it.
 *
 * \returns If there is a failure from which the there is no recovery, returns
 * non-zero, otherwise returns 0.
 *
 * The rest of the parameters are the same as #clang_parseTranslationUnit.
 */
int clang_indexSourceFile(CXIndexAction,
                                         CXClientData client_data,
                                         IndexerCallbacks *index_callbacks,
                                         unsigned index_callbacks_size,
                                         unsigned index_options,
                                         const char *source_filename,
                                         const char * const *command_line_args,
                                         int num_command_line_args,
                                         struct CXUnsavedFile *unsaved_files,
                                         unsigned num_unsaved_files,
                                         CXTranslationUnit *out_TU,
                                         unsigned TU_options);

/**
 * \brief Index the given translation unit via callbacks implemented through
 * #IndexerCallbacks.
 *
 * The order of callback invocations is not guaranteed to be the same as
 * when indexing a source file. The high level order will be:
 *
 *   -Preprocessor callbacks invocations
 *   -Declaration/reference callbacks invocations
 *   -Diagnostic callback invocations
 *
 * The parameters are the same as #clang_indexSourceFile.
 *
 * \returns If there is a failure from which the there is no recovery, returns
 * non-zero, otherwise returns 0.
 */
int clang_indexTranslationUnit(CXIndexAction,
                                              CXClientData client_data,
                                              IndexerCallbacks *index_callbacks,
                                              unsigned index_callbacks_size,
                                              unsigned index_options,
                                              CXTranslationUnit);

/**
 * \brief Retrieve the CXIdxFile, file, line, column, and offset represented by
 * the given CXIdxLoc.
 *
 * If the location refers into a macro expansion, retrieves the
 * location of the macro expansion and if it refers into a macro argument
 * retrieves the location of the argument.
 */
void clang_indexLoc_getFileLocation(CXIdxLoc loc,
                                                   CXIdxClientFile *indexFile,
                                                   CXFile *file,
                                                   unsigned *line,
                                                   unsigned *column,
                                                   unsigned *offset);

