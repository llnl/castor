Require Import Coq.Lists.List.
Import ListNotations.

Notation string := (list Byte.byte) (only parsing).
Definition id_string := @id string.

String Notation string id_string id_string : list_scope.

Section pegs.

  (* Nonterminals *)
  Context (NT: Type).

  Inductive expr := 
    | EAccept : expr
    (* | EReject : expr *)
    | ETerminal: Byte.byte -> expr 
    | ENonterminal: NT -> expr
    | ESequence: expr -> expr -> expr
    | ENegate: expr -> expr
    | ERepeat: expr -> expr
    | EAlt: expr -> expr -> expr.

  Definition grammar := NT -> expr.

  Context (gram: grammar).

  (* Natural parsing semantics: 
    Relate a peg expression e and an input string s to either a parse failure
    or a remainder suffix s'.
   *)

  Inductive parses_nat_expr : expr -> string -> option string -> Prop :=
  | PNEAccept : 
    forall s, 
      parses_nat_expr EAccept s (Some s)
  | PNETermS : 
    forall c s,
      parses_nat_expr (ETerminal c) (c :: s) (Some s)
  | PNETermFne: 
    forall c c' s,
      c <> c' ->
      parses_nat_expr (ETerminal c) (c' :: s) None
  | PNETermFe:
    forall c, 
      parses_nat_expr (ETerminal c) [] None
  | PNENTermR:
    forall v s r,
      parses_nat_expr (gram v) s r -> 
      parses_nat_expr (ENonterminal v) s r
  | PNENTermF:
    forall v s,
      parses_nat_expr (ENonterminal v) s None
  | PNENegS:
    forall e s,
      parses_nat_expr e s None ->
      parses_nat_expr (ENegate e) s (Some s)
  | PNENegF:
    forall e s s',
      parses_nat_expr e s (Some s') ->
      parses_nat_expr (ENegate e) s None
  | PNECatL:
    forall e e' s s' r,
      parses_nat_expr e (s ++ s') (Some s') ->
      parses_nat_expr e' s' r ->
      parses_nat_expr (ESequence e e') s r
  | PNECatR:
    forall e e' s,
      parses_nat_expr e s None ->
      parses_nat_expr (ESequence e e') s None
  | PNEAltL:
    forall e e' s s',
      parses_nat_expr e s (Some s') ->
      parses_nat_expr (EAlt e e') s (Some s')
  | PNEAltR:
    forall e e' s r,
      parses_nat_expr e s None ->
      parses_nat_expr e' s r ->
      parses_nat_expr (ESequence e e') s r
  | PNERepB:
    forall e s,
      parses_nat_expr e s None ->
      parses_nat_expr (ERepeat e) s (Some s)
  | PNERepR:
    forall e s s' s'',
      parses_nat_expr e s (Some s') ->
      parses_nat_expr (ERepeat e) s' (Some s'') ->
      parses_nat_expr (ERepeat e) s (Some s'').

  (* Expression and prefix to remainder *)
  Definition prefix_memoizer := expr -> string -> option string.
  Definition prefix_memo_sound (pref_memo: prefix_memoizer) := 
    forall e s s' s'',
      pref_memo e s = Some s' -> 
      parses_nat_expr e (s ++ s' ++ s'') (Some (s' ++ s'')).

  





  Definition parses_nat_toplevel (start: NT) (s: string) :=
    parses_nat_expr (gram start) s (Some []).

End pegs.