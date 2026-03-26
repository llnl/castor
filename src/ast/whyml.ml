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
        | ArrayType of typ

and typ =
        | UnitType
        | BoolType
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
        | CastExpr of typ * rValue
        | AddressOfExpr of lValue
        | FunctionCallExpr of (name * rValue array)
        | MemoryGetExpr of lValue
        | Literal of literal
        | UnaryOperation of (unaryOperation * rValue)
        | BinaryOperation of (binaryOperation * rValue * rValue)
        | AssignExpr of (lValue * rValue)

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

and expression =
        | LValue of (lValue * typ)
        | RValue of (rValue * typ)

and statement = 
        | Statement of stmt * statement option

and stmt =
        | VariableDeclarationStmt of (name * rValue option)
        | IfStmt of (rValue * statement * statement option)
        | LoopStmt of (statement * rValue * statement * statement * loopvc array)
        | Assertion of assertionvc
        | ReturnStmt of rValue
        | DiscardResultStmt of expression

type func = name * variable array * statement * functionvc array

type globalItem =
        | Function of func
        | Program of func array

type node =
        | Type of typ
        | Expression of expression
        | Statement of statement
        | GlobalItem of globalItem

