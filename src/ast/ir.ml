(*
   Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
   See the top-level LICENSE file for details.

   SPDX-License-Identifier: BSD-3-Clause
*)

type name = string

type loopvc
type assertionvc
type functionvc

type integralType =
        | S64Type
        | U64Type
        | S32Type
        | U32Type
        | S16Type
        | U16Type
        | S8Type
        | U8Type
        | PointerType of typ

and typ =
        | VoidType
        | BoolType
        | NonRealType
        | ArrayType of typ
        | ClassType of name
        | IntegralType of integralType

type variable = name * typ

type lValue =
        | PointerDereference of expression
        | VariableReference of variable
        | ArrayIndex of (lValue * expression)
        | FieldReference of (lValue * name)

and functionRefExpr =
        | FunctionRefExpr of name

and memberFunctionRefExpr =
        | MemberFunctionRefExpr of (lValue * name)

and templateFunctionParameter =
        | ExpressionParam of expression
        | TypeParam of typ

and rValue =
        | SizeOfExpr of typ
        | CastExpr of typ * expression
        | AddressOfExpr of lValue
        | FunctionCallExpr of (functionRefExpr * expression array)
        | MemberFunctionCallExpr of (memberFunctionRefExpr * expression array)
        | TemplateFunctionCallExpr of
         (functionRefExpr * expression array * templateFunctionParameter array)
        | TemplateMemberFunctionCallExpr of
         (memberFunctionRefExpr * expression array * templateFunctionParameter array)
        | Literal of literal
        | UnaryOperation of (unaryOperation * expression)
        | BinaryOperation of (binaryOperation * expression * expression)

and literal =
        | S64Literal of int
        | U64Literal of int
        | S32Literal of int
        | U32Literal of int
        | S16Literal of int
        | U16Literal of int
        | S8Literal of int
        | U8Literal of int
        | BoolLiteral of bool

and unaryOperation =
        | BitNegationOp
        | NegationOp

and binaryOperation =
        | AdditionOp
        | SubtractionOp
        | BitAndOp
        | BitOrOp
        | BitLShiftOp
        | BitRShiftOp
        | EqualsOp
        | NotEqualsOp
        | MultiplyOp
        | DivideOp
        | LessThanOp
        | LessEqualsOp
        | GreaterThanOp
        | GreaterEqualsOp
        | AssignOp

and expression =
        | LValue of (lValue * typ)
        | RValue of (rValue * typ)

type caseStmt = literal * statement array

and defaultStmt = statement array

and statementBlock = statement array

and statement =
        | VariableDeclarationStmt of (name * expression option)
        | BreakStmt
        | IfStmt of (expression * statement * statement option)
        | LoopStmt of (expression * expression * expression * statement * loopvc array)
        | CaseStmt of caseStmt
        | DefaultStmt of defaultStmt
        | Assertion of assertionvc
        | SwitchStmt of (expression * caseStmt array * defaultStmt option)
        | StatementBlock of statementBlock
        | ReturnStmt of expression
        | ExpressionStmt of expression

type func = name * variable array * statementBlock * functionvc array

type clas = name * variable array * func array

type file = globalItem array

and globalItem =
        | Function of func
        | TemplateFunction of func * variable array
        | Class of clas
        | TemplateClass of clas * variable array
        | File of file
        | Program of file array

type node =
        | Type of typ
        | Expression of expression
        | Statement of statement
        | GlobalItem of globalItem

