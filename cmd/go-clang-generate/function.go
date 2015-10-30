package main

import (
	"bytes"
	"go/ast"
	"go/format"
	"go/token"
	"strings"
	"text/template"

	"github.com/sbinet/go-clang"
)

type Function struct {
	Name    string
	CName   string
	Comment string

	Parameters []FunctionParameter
	ReturnType Type

	Receiver Receiver

	Member string
}

type FunctionParameter struct {
	Name  string
	CName string
	Type  Type
}

func handleFunctionCursor(cursor clang.Cursor) *Function {
	f := Function{
		CName:   cursor.Spelling(),
		Comment: cleanDoxygenComment(cursor.RawCommentText()),

		Parameters: []FunctionParameter{},
	}

	f.Name = strings.TrimPrefix(f.CName, "clang_")

	typ, err := getType(cursor.ResultType())
	if err != nil {
		panic(err)
	}
	f.ReturnType = typ

	numParam := uint(cursor.NumArguments())
	for i := uint(0); i < numParam; i++ {
		param := cursor.Argument(i)

		p := FunctionParameter{
			CName: param.DisplayName(),
		}

		p.Name = p.CName
		typ, err := getType(param.Type())
		if err != nil {
			panic(err)
		}
		p.Type = typ

		if p.Name == "" {
			p.Name = receiverName(p.Type.Name)
		}

		f.Parameters = append(f.Parameters, p)
	}

	return &f
}

func generateASTFunction(f *Function) string {
	astFunc := ast.FuncDecl{
		Name: &ast.Ident{
			Name: f.Name,
		},
		Type: &ast.FuncType{
			Results: &ast.FieldList{
				List: []*ast.Field{},
			},
		},
		Body: &ast.BlockStmt{},
	}

	retur := &ast.ReturnStmt{
		Results: []ast.Expr{},
	}
	hasReturnArguments := false

	accessMember := func(variable string, member string) *ast.SelectorExpr {
		return &ast.SelectorExpr{
			X: &ast.Ident{
				Name: variable,
			},
			Sel: &ast.Ident{
				Name: member,
			},
		}
	}
	addStatement := func(stmt ast.Stmt) {
		astFunc.Body.List = append(astFunc.Body.List, stmt)
	}
	addAssignment := func(variable string, e ast.Expr) {
		addStatement(&ast.AssignStmt{
			Lhs: []ast.Expr{
				&ast.Ident{
					Name: variable,
				},
			},
			Tok: token.DEFINE,
			Rhs: []ast.Expr{
				e,
			},
		})
	}
	addAssignmentToO := func(e ast.Expr) {
		addAssignment("o", e)
	}
	addDefer := func(call *ast.CallExpr) {
		addStatement(&ast.DeferStmt{
			Call: call,
		})
	}
	addEmptyLine := func() {
		// TODO this should be done using something else than a fake statement.
		addStatement(&ast.ExprStmt{
			X: &ast.CallExpr{
				Fun: &ast.Ident{
					Name: "REMOVE",
				},
			},
		})
	}
	addReturnItem := func(item ast.Expr) {
		retur.Results = append(retur.Results, item)
	}
	doCall := func(variable string, method string, args ...ast.Expr) *ast.CallExpr {
		return &ast.CallExpr{
			Fun:  accessMember(variable, method),
			Args: args,
		}
	}
	doCast := func(typ string, args ...ast.Expr) *ast.CallExpr {
		return &ast.CallExpr{
			Fun: &ast.Ident{
				Name: typ,
			},
			Args: args,
		}
	}
	doCompose := func(typ string, v ast.Expr) *ast.CompositeLit {
		return &ast.CompositeLit{
			Type: &ast.Ident{
				Name: typ,
			},
			Elts: []ast.Expr{
				v,
			},
		}
	}
	doCType := func(c string) *ast.SelectorExpr {
		return accessMember("C", c)
	}
	doCCast := func(typ string, args ...ast.Expr) *ast.CallExpr {
		return doCall("C", typ, args...)
	}
	doField := func(name string, typ Type) *ast.Field {
		f := &ast.Field{}

		if name != "" {
			f.Names = []*ast.Ident{
				&ast.Ident{
					Name: name,
				},
			}
		}
		if typ.Name != "" {
			if typ.PointerLevel > 0 && typ.CName == CSChar {
				f.Type = &ast.Ident{
					Name: "string",
				}
			} else {
				f.Type = &ast.Ident{
					Name: typ.Name,
				}
			}

			if typ.IsSlice {
				f.Type = &ast.ArrayType{
					Elt: f.Type,
				}
			}
		}

		return f
	}
	addReturnType := func(name string, typ Type) {
		astFunc.Type.Results.List = append(astFunc.Type.Results.List, doField(name, typ))
	}
	doZero := func() *ast.BasicLit {
		return &ast.BasicLit{
			Kind:  token.INT,
			Value: "0",
		}
	}

	// TODO maybe name the return arguments ... because of clang_getDiagnosticOption -> the normal return can be always just "o"?

	// TODO reenable this, see the comment at the bottom of the generate function for details
	// Add function comment
	/*if f.Comment != "" {
		astFunc.Doc = &ast.CommentGroup{
			List: []*ast.Comment{
				&ast.Comment{
					Text: f.Comment,
				},
			},
		}
	}*/

	// Add receiver to make function a method
	if f.Receiver.Name != "" {
		if len(f.Parameters) > 0 { // TODO maybe to not set the receiver at all? -> do this outside of the generate function?
			astFunc.Recv = &ast.FieldList{
				List: []*ast.Field{
					doField(f.Receiver.Name, f.Receiver.Type),
				},
			}
		}
	}

	// Basic call to the C function
	call := doCCast(f.CName)

	if len(f.Parameters) != 0 {
		if f.Receiver.Name != "" {
			f.Parameters[0].Name = f.Receiver.Name
		}

		astFunc.Type.Params = &ast.FieldList{
			List: []*ast.Field{},
		}

		hasDeclaration := false

		// Add parameters to the function
		for i, p := range f.Parameters {
			if i == 0 && f.Receiver.Name != "" {
				continue
			}

			// Ingore length parameters since they will be filled by the slice itself
			if p.Type.LengthOfSlice != "" {
				continue
			}

			if p.Type.IsSlice { // TODO think about doing slice return arguments
				hasDeclaration = true

				// Declare the slice
				var sliceType ast.Expr

				if p.Type.PointerLevel > 0 && p.Type.CName == CSChar {
					sliceType = doCType("char")
				} else {
					sliceType = doCType(p.Type.Primitive)
				}

				for i := 1; i < p.Type.PointerLevel; i++ {
					sliceType = &ast.StarExpr{
						X: sliceType,
					}
				}

				addAssignment(
					"ca_"+p.Name,
					doCast(
						"make",
						&ast.ArrayType{
							Elt: sliceType,
						},
						doCast(
							"len",
							&ast.Ident{
								Name: p.Name,
							},
						),
					),
				)
				addStatement(&ast.DeclStmt{
					Decl: &ast.GenDecl{
						Tok: token.VAR,
						Specs: []ast.Spec{
							&ast.ValueSpec{
								Names: []*ast.Ident{
									&ast.Ident{
										Name: "cp_" + p.Name,
									},
								},
								Type: &ast.StarExpr{
									X: sliceType,
								},
							},
						},
					},
				})
				addStatement(&ast.IfStmt{
					Cond: &ast.BinaryExpr{
						X: doCast(
							"len",
							&ast.Ident{
								Name: p.Name,
							},
						),
						Op: token.GTR,
						Y:  doZero(),
					},
					Body: &ast.BlockStmt{
						List: []ast.Stmt{
							&ast.AssignStmt{
								Lhs: []ast.Expr{
									&ast.Ident{
										Name: "cp_" + p.Name,
									},
								},
								Tok: token.ASSIGN,
								Rhs: []ast.Expr{
									&ast.UnaryExpr{
										Op: token.AND,
										X: &ast.IndexExpr{
											X: &ast.Ident{
												Name: "ca_" + p.Name,
											},
											Index: doZero(),
										},
									},
								},
							},
						},
					},
				})

				// Assign elements
				var loopStatements []ast.Stmt

				// Handle our good old friend the const char * differently...
				if p.Type.CName == CSChar {
					loopStatements = append(loopStatements, &ast.AssignStmt{
						Lhs: []ast.Expr{
							&ast.Ident{
								Name: "ci_str",
							},
						},
						Tok: token.DEFINE,
						Rhs: []ast.Expr{
							doCCast(
								"CString",
								&ast.IndexExpr{
									X: &ast.Ident{
										Name: p.Name,
									},
									Index: &ast.Ident{
										Name: "i",
									},
								},
							),
						},
					})
					loopStatements = append(loopStatements, &ast.DeferStmt{
						Call: doCCast(
							"free",
							doCall(
								"unsafe",
								"Pointer",
								&ast.Ident{
									Name: "ci_str",
								},
							),
						),
					})
					loopStatements = append(loopStatements, &ast.AssignStmt{
						Lhs: []ast.Expr{
							&ast.IndexExpr{
								X: &ast.Ident{
									Name: "ca_" + p.Name,
								},
								Index: &ast.Ident{
									Name: "i",
								},
							},
						},
						Tok: token.ASSIGN,
						Rhs: []ast.Expr{
							&ast.Ident{
								Name: "ci_str",
							},
						},
					})
				} else {
					loopStatements = append(loopStatements, &ast.AssignStmt{
						Lhs: []ast.Expr{
							&ast.IndexExpr{
								X: &ast.Ident{
									Name: "ca_" + p.Name,
								},
								Index: &ast.Ident{
									Name: "i",
								},
							},
						},
						Tok: token.ASSIGN,
						Rhs: []ast.Expr{
							&ast.SelectorExpr{
								X: &ast.IndexExpr{
									X: &ast.Ident{
										Name: p.Name,
									},
									Index: &ast.Ident{
										Name: "i",
									},
								},
								Sel: &ast.Ident{
									Name: "c",
								},
							},
						},
					})
				}

				addStatement(&ast.RangeStmt{
					Key: &ast.Ident{
						Name: "i",
					},
					Tok: token.DEFINE,
					X: &ast.Ident{
						Name: p.Name,
					},
					Body: &ast.BlockStmt{
						List: loopStatements,
					},
				})
			} else if p.Type.IsReturnArgument {
				hasReturnArguments = true

				// Add the return type to the function return arguments
				var retType string
				if p.Type.Name == "cxstring" {
					retType = "string"
				} else {
					retType = p.Type.Name
				}

				addReturnType("", Type{
					Name: retType,
				})

				// Declare the return argument's variable
				var varType ast.Expr
				if p.Type.Primitive != "" {
					varType = doCType(p.Type.Primitive)
				} else {
					varType = &ast.Ident{
						Name: p.Type.Name,
					}
				}
				if p.Type.IsSlice {
					varType = &ast.ArrayType{
						Elt: varType,
					}
				}

				addStatement(&ast.DeclStmt{
					Decl: &ast.GenDecl{
						Tok: token.VAR,
						Specs: []ast.Spec{
							&ast.ValueSpec{
								Names: []*ast.Ident{
									&ast.Ident{
										Name: p.Name,
									},
								},
								Type: varType,
							},
						},
					},
				})
				if p.Type.Name == "cxstring" {
					addDefer(doCall(p.Name, "Dispose"))
				}

				// Add the return argument to the return statement
				if p.Type.Primitive != "" {
					addReturnItem(doCast(
						p.Type.Name,
						&ast.Ident{
							Name: p.Name,
						},
					))
				} else {
					if p.Type.Name == "cxstring" {
						addReturnItem(doCall(p.Name, "String"))
					} else {
						addReturnItem(&ast.Ident{
							Name: p.Name,
						})
					}
				}

				continue
			}

			astFunc.Type.Params.List = append(astFunc.Type.Params.List, doField(p.Name, p.Type))
		}

		if hasReturnArguments || hasDeclaration {
			addEmptyLine()
		}

		goToCTypeConversions := false

		// Add arguments to the C function call
		for _, p := range f.Parameters {
			var pf ast.Expr

			if p.Type.IsSlice {
				pf = &ast.Ident{
					Name: "cp_" + p.Name,
				}
			} else if p.Type.Primitive != "" {
				// Handle Go type to C type conversions
				if p.Type.PointerLevel == 1 && p.Type.CName == CSChar {
					goToCTypeConversions = true

					addAssignment(
						"c_"+p.Name,
						doCCast(
							"CString",
							&ast.Ident{
								Name: p.Name,
							},
						),
					)
					addDefer(doCCast(
						"free",
						doCall(
							"unsafe",
							"Pointer",
							&ast.Ident{
								Name: "c_" + p.Name,
							},
						),
					))

					pf = &ast.Ident{
						Name: "c_" + p.Name,
					}
				} else if p.Type.Primitive == "cxstring" { // TODO try to get cxstring and "String" completely out of this function since it is just a struct which can be handled by the struct code
					pf = accessMember(p.Name, "c")
				} else {
					if p.Type.IsReturnArgument {
						// Return arguments already have a cast
						pf = &ast.Ident{
							Name: p.Name,
						}
					} else if p.Type.LengthOfSlice != "" {
						pf = doCCast(
							p.Type.Primitive,
							doCast(
								"len",
								&ast.Ident{
									Name: p.Type.LengthOfSlice,
								},
							),
						)
					} else {
						pf = doCCast(
							p.Type.Primitive,
							&ast.Ident{
								Name: p.Name,
							},
						)
					}
				}
			} else {
				pf = accessMember(p.Name, "c")
			}

			if p.Type.IsReturnArgument {
				pf = &ast.UnaryExpr{
					Op: token.AND,
					X:  pf,
				}
			}

			call.Args = append(call.Args, pf)
		}

		if goToCTypeConversions {
			addEmptyLine()
		}
	}

	// Check if we need to add a return
	if f.ReturnType.Name != "void" || hasReturnArguments {
		if f.ReturnType.Name == "cxstring" {
			// Do the C function call and save the result into the new variable "o" while transforming it into a cxstring
			addAssignmentToO(doCompose("cxstring", call))
			addDefer(doCall("o", "Dispose"))
			addEmptyLine()

			// Call the String method on the cxstring instance
			addReturnItem(doCall("o", "String"))

			// Change the return type to "string"
			addReturnType("", Type{
				Name: "string",
			})
		} else {
			if f.ReturnType.Name != "void" {
				// Add the function return type
				addReturnType("", f.ReturnType)
			}

			// Do we need to convert the return of the C function into a boolean?
			if f.ReturnType.Name == "bool" && f.ReturnType.Primitive != "" {
				// Do the C function call and save the result into the new variable "o"
				addAssignmentToO(call)
				addEmptyLine()

				// Check if o is not equal to zero and return the result
				addReturnItem(&ast.BinaryExpr{
					X: &ast.Ident{
						Name: "o",
					},
					Op: token.NEQ,
					Y: doCCast(
						f.ReturnType.Primitive,
						doZero(),
					),
				})
			} else if f.ReturnType.CName == CSChar && f.ReturnType.PointerLevel == 1 {
				// If this is a normal const char * C type there is not so much to do
				addReturnItem(doCCast(
					"GoString",
					call,
				))
			} else if f.ReturnType.Name == "time.Time" {
				addReturnItem(doCall(
					"time",
					"Unix",
					doCast("int64", call),
					doZero(),
				))
			} else if f.ReturnType.Name == "void" {
				// Handle the case where the C function has no return argument but parameters that are return arguments

				// Do the C function call
				addStatement(&ast.ExprStmt{
					X: call,
				})
				addEmptyLine()
			} else {
				var convCall ast.Expr

				// Structs are literals, everything else is a cast
				if f.ReturnType.Primitive == "" {
					convCall = doCompose(f.ReturnType.Name, call)
				} else {
					convCall = doCast(f.ReturnType.Name, call)
				}

				if hasReturnArguments {
					// Do the C function call and save the result into the new variable "o"
					addAssignmentToO(convCall)
					addEmptyLine()

					// Add the C function call result to the return statement
					addReturnItem(&ast.Ident{
						Name: "o",
					})
				} else {
					addReturnItem(convCall)
				}
			}
		}

		// Add the return statement
		addStatement(retur)
	} else {
		// No return needed, just add the C function call
		addStatement(&ast.ExprStmt{
			X: call,
		})
	}

	var b bytes.Buffer
	err := format.Node(&b, token.NewFileSet(), []ast.Decl{&astFunc})
	if err != nil {
		panic(err)
	}

	sss := b.String()

	// TODO hack to make new lines...
	sss = strings.Replace(sss, "REMOVE()", "", -1)

	// TODO find out how to position the comment correctly and do this using the AST
	if f.Comment != "" {
		sss = f.Comment + "\n" + sss
	}

	return sss
}

var templateGenerateStructMemberGetter = template.Must(template.New("go-clang-generate-function-getter").Parse(`{{$.Comment}}
func ({{$.Receiver.Name}} {{$.Receiver.Type.Name}}) {{$.Name}}() {{if ge $.ReturnType.PointerLevel 1}}*{{end}}{{$.ReturnType.Name}} {
	value := {{if eq $.ReturnType.Name "bool"}}{{$.Receiver.Name}}.c.{{$.Member}}{{else}}{{$.ReturnType.Name}}{{if $.ReturnType.IsPrimitive}}({{if ge $.ReturnType.PointerLevel 1}}*{{end}}{{$.Receiver.Name}}.c.{{$.Member}}){{else}}{{"{"}}{{if ge $.ReturnType.PointerLevel 1}}*{{end}}{{$.Receiver.Name}}.c.{{$.Member}}{{"}"}}{{end}}{{end}}
	return {{if eq $.ReturnType.Name "bool"}}value != C.int(0){{else}}{{if ge $.ReturnType.PointerLevel 1}}&{{end}}value{{end}}
}
`))

func generateFunctionStructMemberGetter(f *Function) string {
	var b bytes.Buffer
	if err := templateGenerateStructMemberGetter.Execute(&b, f); err != nil {
		panic(err)
	}

	return b.String()
}

type FunctionSliceReturn struct {
	Function

	SizeMember string

	CElementType    string
	ElementType     string
	IsPrimitive     bool
	ArrayDimensions int
	ArraySize       int64
}

var templateGenerateReturnSlice = template.Must(template.New("go-clang-generate-slice").Parse(`{{$.Comment}}
func ({{$.Receiver.Name}} {{$.Receiver.Type.Name}}) {{$.Name}}() []{{if eq $.ArrayDimensions 2 }}*{{end}}{{$.ElementType}} {
	sc := []{{if eq $.ArrayDimensions 2 }}*{{end}}{{$.ElementType}}{}

	length := {{if ne $.ArraySize -1}}{{$.ArraySize}}{{else}}int({{$.Receiver.Name}}.c.{{$.SizeMember}}){{end}}
	goslice := (*[1 << 30]{{if or (eq $.ArrayDimensions 2) (eq $.ElementType "unsafe.Pointer")}}*{{end}}C.{{$.CElementType}})(unsafe.Pointer(&{{$.Receiver.Name}}.c.{{$.Member}}))[:length:length]

	for is := 0; is < length; is++ {
		sc = append(sc, {{if eq $.ArrayDimensions 2}}&{{end}}{{$.ElementType}}{{if $.IsPrimitive}}({{if eq $.ArrayDimensions 2}}*{{end}}goslice[is]){{else}}{{"{"}}{{if eq $.ArrayDimensions 2}}*{{end}}goslice[is]{{"}"}}{{end}})
	}

	return sc
}
`))

func generateFunctionSliceReturn(f *FunctionSliceReturn) string {
	var b bytes.Buffer
	if err := templateGenerateReturnSlice.Execute(&b, f); err != nil {
		panic(err)
	}

	return b.String()

}

func generateFunction(name, cname, comment, member string, typ Type) *Function {
	receiverType := trimClangPrefix(cname)
	receiverName := receiverName(receiverType)
	functionName := upperFirstCharacter(name)

	if typ.IsPrimitive {
		typ.Primitive = typ.Name
	}
	if (strings.HasPrefix(name, "has") || strings.HasPrefix(name, "is")) && typ.Name == GoInt16 {
		typ.Name = GoBool
	}

	f := &Function{
		Name:    functionName,
		CName:   cname,
		Comment: comment,

		Parameters: []FunctionParameter{},

		ReturnType: typ,
		Receiver: Receiver{
			Name: receiverName,
			Type: Type{
				Name: receiverType,
			},
		},

		Member: member,
	}

	return f
}
