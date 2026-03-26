(*
   Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
   See the top-level LICENSE file for details.

   SPDX-License-Identifier: BSD-3-Clause
*)

type name = string

type expression =
        | TruthFunctionalExpr of truthFunctionalExpr
        | Value of value

and truthFunctionalExpr =
        | BinOps of (binOps * truthFunctionalExpr * truthFunctionalExpr)
        | Negation of truthFunctionalExpr
        | Quantifier of (quantifier * value array * truthFunctionalExpr)
        | PredicateCall of (name * value array)
        | ComparisonOperator of (comparisonOperator * value * value)

and binOps =
        | Conjunction
        | Disjunction
        | Implication
        | MutualImplication

and quantifier =
        | Universal
        | Existential

and comparisonOperator = 
        | Equals
        | NotEquals
        | GreaterThan
        | GreaterEquals
        | LessThan
        | LessEquals

and value = 
        | BinOps of valueBinOps
        | Negation of value
        | WriteableValue of writeableValue
        | OldValue of value
        | AtValue of value * name
        | Literal of int

and valueBinOps =
        | Add
        | Subtract
        | Multiply
        | Divide

and writeableValue =
        | VariableReference of variable
        | PointerDereference of writeableValue

and variable = name * typ

and typ

type vcs =
        | LoopVC of loopvc
        | FunctionVC of functionvc
        | AssertionVC of assertionvc

and loopvc = 
        | LoopVariant of value
        | LoopInvariant of truthFunctionalExpr
        | Writes of writeableValue array

and functionvc =
        | Writes of writeableValue array
        | Requires of truthFunctionalExpr
        | Ensures of truthFunctionalExpr
        | Valid of writeableValue array
        | Separated of writeableValue array

and assertionvc =
        | Assertion of truthFunctionalExpr
