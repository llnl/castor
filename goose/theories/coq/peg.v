Require Import Coq.Lists.List.
Import ListNotations.

Notation string := (list Byte.byte) (only parsing).
Definition id_string := @id string.

String Notation string id_string id_string : list_scope.

Section pegs.

  (* Nonterminals *)
  Context (NT: Type).

  Inductive expr := 
    (* | EAccept : expr *)
    | EReject : expr
    | ETerminal: string -> expr 
    | ENonterminal: NT -> expr
    | ESequence: expr -> expr -> expr
    | ENegate: expr -> expr
    | ERepeat: expr -> expr
    | EAlt: expr -> expr -> expr.

  Definition accept := ETerminal [].

  Fixpoint concat (ss: list string) : string := 
    match ss with 
    | [] => []
    | s :: ss => 
      s ++ concat ss
    end.

  Definition grammar := NT -> expr.

  (* 
    Expr <- (Term '+' Term / Term '-' Term / Term)
    Term <- 1 / Expr 

    Inductive nt := ex | t.
    Definition grammar (v: nt) : expr :=
      match v with 
      | ex => (EAlt (EAlt ...))
      | t => EAlt (ETerminal "1") (ENonterminal ex)
      end.
  
   *)

  Context (gram: grammar).

  Fixpoint parses_expr (fuel: nat) (s: string) (e: expr) : Prop :=
    match fuel with 
    | 0 => False
    | S fuel => 
      match e with
      | EReject => False
      | ETerminal s' => s = s'
      | ESequence e1 e2 =>
          exists s1 s2,
          s = s1 ++ s2 /\ parses_expr fuel s1 e1 /\ parses_expr fuel s2 e2
      | ENegate e' => 
        ~ parses_expr fuel s e'
      | ENonterminal nt => 
        parses_expr fuel s (gram nt)
      | ERepeat e =>
          exists l, 
            s = concat l /\
            forall s', In s' l -> parses_expr fuel s' e
      | EAlt e1 e2 =>
          parses_expr fuel s e1 \/ (~ parses_expr fuel s e1 /\ parses_expr fuel s e2)
      end
    end.

  Definition parses_toplevel (start: NT) (fuel: nat) (s: string) :=
    parses_expr fuel s (gram start).

End pegs.

Section pegs_actions.

  (* Nonterminals, which are typed by their return type. 
     This setup is similar to a typed version of HOAS, i.e.,
     variables are indexed by their types.
  *)
  Context (NT: Type -> Type).

  (* Intuition: index the expr type by the return value of the action monad *)
  Inductive expr_action : Type -> Type := 
    (* These two cases are like monadic pure and monadic bind *)
    | EAAccept : forall {A}, A -> expr_action A
    (* In parsers, monadic bind replaces sequence. *)
    | EABind : forall {A B},
      expr_action A -> (A -> expr_action B) -> expr_action B

    (* Reject behaves like fail in the error monad. *)
    | EAReject : forall {A}, expr_action A

    (* PEG parser specific primitives. *)
    | EATerminal: string -> expr_action string 
    (* Nonterminals look like HOAS variables. *)
    | EANonterminal: forall {A}, NT A -> expr_action A
    | EANegate: forall {A}, expr_action A -> expr_action unit
    | EARepeat: forall {A}, expr_action A -> expr_action (list A)
    | EAAlt: forall {A}, expr_action A -> expr_action A -> expr_action A.

  Definition grammar_action := forall A, NT A -> expr_action A.

  Fixpoint parses_expr_action
     (gram: forall A, NT A -> expr_action A)
     (fuel: nat) (s: string) {A: Type} (e: expr_action A) (a: A) : Prop
  :=
  match fuel with | 0 => False
  | S fuel0 =>
      match e in expr_action V return V -> Prop with
      | EAReject => fun _ => False
      | EAAccept ea => fun v => ea = v
      | EANonterminal nta => parses_expr_action gram fuel0 s (gram _ nta)
      | EAAlt e1 e2 => fun v =>
          let e1_output := parses_expr_action gram fuel0 s e1 v in
          e1_output \/ (~(e1_output) /\ parses_expr_action gram fuel0 s e2 v)
      | EATerminal s0 => fun _ => s0 = s 
      | EARepeat ear => fun v =>
          exists l,
          s = concat l
          /\ List.length l = List.length v
          /\ (forall plv, In plv (List.combine l v) -> parses_expr_action gram fuel0 (fst plv) ear (snd plv))
      | EABind ea aeb => fun v =>
          exists s1 s2,
          s = s1 ++ s2 /\
          (exists intermediary,
            (parses_expr_action gram fuel0 s1 ea intermediary)
            /\
            (parses_expr_action gram fuel0 s2 (aeb intermediary) v))
      | EANegate ea => fun _ => forall v, ~ (parses_expr_action gram fuel0 s ea v)
      end a
  end.

  Definition parses_toplevel_action {A: Type}
    (gram: forall A, NT A -> expr_action A)
    (start: NT A) (fuel: nat) (s: string) (a: A)
  := parses_expr_action gram fuel s (gram _ start) a.

  (* Concatenation or sequence: given parsers for A and B, sequence them together
     to make a parser for (A * B) 
     
  *)
  Definition cat {A B} (l : expr_action A) (r: expr_action B) : expr_action (A * B) := 
    EABind l (fun x => EABind r (fun y => EAAccept (x, y))).

  (* 
    Map applies a function to a parser
   *)

  Definition map {A B} (x: expr_action A) (f: A -> B) := 
    EABind x (fun v => EAAccept (f v)).

  (* Shorthand aliases for the primitives because John likes "pure" instead of "return". *)
  Definition pure {A} := @EAAccept A.
  Definition fail {A} := @EAReject A.

  Definition term := EATerminal.

End pegs_actions.

Arguments EAAccept {_ _} _.
Arguments EAReject {_ _}.
Arguments EATerminal {_} _.
Arguments EANonterminal {_ _} _.
Arguments EABind {_ _ _} _.
Arguments EANegate {_ _} _.
Arguments EARepeat {_ _} _.
Arguments EAAlt {_ _} _ _.

Arguments cat {_ _ _} _ _.
Arguments map {_ _ _} _ _.
Arguments pure {_ _} _.
Arguments fail {_ _}.
Arguments term {_}.

Infix "$" := (cat) (at level 80).
Infix "@" := (map) (at level 80).
Infix "//" := (EAAlt) (at level 49, left associativity).
Infix ">>=" := (EABind) (at level 80).

Notation "p1 $> p2" := (cat p1 p2 @ fun '(_,x) => x) (at level 81).
Notation "p1 <$ p2" := (cat p1 p2 @ fun '(x,_) => x) (at level 50).
Notation "'let*' ' x := g 'in' f " := (g >>= (fun x => f) ) (at level 20, x pattern, f at next level).
Notation "'let*'  x := g 'in' f " := (g >>= (fun x => f)) (at level 20, x name, f at next level).
Notation " c1 ';;' c2 " := (let* _ := c1 in c2 ) (at level 19, right associativity).


(* 
  parse the following:

  I -> 1 / 2 / 33           @ to_int
  B -> true / false     @ to_bool
  E -> B / I            @ to_value
  
 *)


Definition valu : Type := (nat + bool).
Inductive int_bool : Type -> Type := i : int_bool nat | b : int_bool bool | v: int_bool valu.

Definition gram_int : expr_action int_bool nat := 
  (* Parse the string "1", throw away the result, return 1 *)
  ((term "1") $> pure 1)
   // 
  ((term "2") $> pure 2)
  // 
  (term "3" $ term "3" $> pure 33).

Definition gram_bool : expr_action int_bool bool := 
  (term "true") $> pure true
   // 
  ((term "false") $> pure false).

Definition spacing : expr_action int_bool unit := 
  EARepeat (term " ") $> pure tt.

Definition four_bools: expr_action int_bool (bool * bool * bool * bool) := 
  gram_bool <$ spacing $ gram_bool $ gram_bool $ gram_bool.

Definition gram_valu : expr_action int_bool valu := 
  (EANonterminal i @ inl) // (EANonterminal b @ inr).

Definition grammar_ib_actions : grammar_action int_bool := 
  fun _ nt => 
  match nt with 
  | i => gram_int
  | b => gram_bool
  | v => gram_valu
  end.

(* An example with monadic bind *)

Definition gram_sum : expr_action int_bool nat := 
  let* x := gram_int in
  spacing ;;  
  let* y := gram_int in 
  spacing ;;
    pure (x + y).

