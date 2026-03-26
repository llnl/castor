Require Import Coq.Lists.List.
Import ListNotations.

From Pegs Require Import peg.

Print grammar.
Print peg.grammar.

Inductive nt := ex | t. Check nt. Check (ENonterminal).
Check ETerminal nt "1".

Definition example_grammar v : expr nt :=
  match v with
  | ex => EAlt nt (ETerminal nt "1") (ENonterminal nt t)
  | t => (ETerminal nt "2")
  end.

Compute parses_toplevel nt example_grammar ex 0 "1".

Check True.

Example example_grammar_parses_1 : parses_expr nt example_grammar 1 "1" (ETerminal nt "1").
Proof. simpl. reflexivity. Qed.

Check parses_toplevel nt example_grammar ex 2 "2".

Compute parses_toplevel nt example_grammar ex 2 "2".

Example example_grammar_parses_1_top: parses_toplevel nt example_grammar ex 2 "1".
Proof. unfold parses_toplevel. unfold parses_expr. simpl. left. reflexivity. Qed.

Example example_grammar_parses_toplevel_2: parses_toplevel nt example_grammar ex 3 "2".
Proof. unfold parses_toplevel. simpl. right. split.
  - unfold "<>". intro. discriminate.
  - reflexivity.
Qed.

Example example_grammar_parses_toplevel_2_exists: exists n, parses_toplevel nt example_grammar ex n "2".
Proof.
  unfold parses_toplevel.
  exists 3.
  simpl. right.
  split.
  - unfold "<>". intro. discriminate.
  - reflexivity.
Qed.

(*
  Expr <- Num '-' Expr / Num '+' Expr / Num
  Num <- '0' / '1' / '2'
*)
Inductive calc_nt := calc_expr | calc_num.

Definition calc_grammar (v: calc_nt) : expr calc_nt :=
  match v with
  | calc_num => EAlt calc_nt (ETerminal calc_nt "0")  (* rule 0 *)
               (EAlt calc_nt (ETerminal calc_nt "1") (* rule 1 *)
                     (ETerminal calc_nt "2")) (* rule 2 *)
  | calc_expr => EAlt calc_nt (ESequence calc_nt 
            (ESequence calc_nt (ENonterminal calc_nt calc_num) (ETerminal calc_nt "-")) (ENonterminal calc_nt calc_num))
            (EAlt calc_nt (ESequence calc_nt
                  (ESequence calc_nt (ENonterminal calc_nt calc_num) 
                  (ETerminal calc_nt "+"))
               (ENonterminal calc_nt calc_num))
            (ENonterminal calc_nt calc_num))
  end.

Definition calc_grammar_proof : grammar calc_nt.
Proof.
  unfold grammar.
  intros.
  destruct H eqn:?.
  - apply EAlt.
    * apply ESequence.
      { apply ESequence.
          apply ENonterminal. apply calc_num.
          apply ETerminal. apply "-".
      } apply ENonterminal. apply calc_expr.
    * apply EAlt.
      { apply ESequence.
          apply ESequence.
            apply ENonterminal. apply calc_num.
            apply ETerminal. apply "+".
          apply ENonterminal. apply calc_expr.
      } apply ENonterminal. apply calc_num.
  - apply EAlt.
    apply EAlt.
      apply ETerminal. apply "0".
      apply ETerminal. apply "1".
      apply ETerminal. apply "2".
Defined.

Print calc_grammar_proof.

Check ESequence _
     (ESequence _ (ENonterminal _ calc_num) (ETerminal _ "-")) (ENonterminal _ calc_expr).

Example calc_grammar_parses_0_exists: exists n, parses_toplevel _ calc_grammar_proof calc_num n "0".
Proof.
  unfold parses_toplevel. simpl.
  unfold parses_expr. simpl.
  exists 4.
  simpl. left. left. reflexivity.
Qed.

Example calc_grammar_parses_1_exists: exists n, parses_toplevel _ calc_grammar calc_num n "1".
Proof.
  unfold parses_toplevel. unfold parses_expr. simpl.
  exists 4.
  right.
  split.
  - unfold "<>". intro. discriminate.
  - left. reflexivity.
Qed.

(* Example calc_grammar_parses_toplevel_0+1_exists: exists n, parses_toplevel _ calc_grammar calc_expr n "0+1". *)
Example calc_grammar_parses_toplevel_0_plus_1_exists: exists n, parses_toplevel _ calc_grammar calc_expr n "0-1".
Proof.
  unfold parses_toplevel.
  exists 6. simpl.
  left.
  exists "0-", "1".
  split.
  - simpl. reflexivity.
  - split.
    * exists "0".
      exists "-".
      split.
        simpl. reflexivity.
        split.
          left. reflexivity.
          reflexivity.
    * right.
      split.
        compute. intro. discriminate.
        left. reflexivity.
Qed.

Example calc_grammar_parses_toplevel_0_exists: exists n, parses_toplevel _ calc_grammar calc_expr n "0".
Proof.
  unfold parses_toplevel. simpl.
  exists 5. simpl.
  right.
  split.
  - unfold "~". intro. destruct H. destruct H. destruct H. destruct H0. destruct H0. destruct H0. destruct H0. destruct H2. destruct H2. assumption. destruct H2. assumption.
  - right.
      split.
      * unfold "~". intro. destruct H. destruct H. destruct H. destruct H0. destruct H1. assumption. destruct H1. assumption.
      * left. reflexivity.
Qed.

Example calc_grammar_parses_toplevel_2_plus_2_exists:
  exists n, parses_toplevel _ calc_grammar calc_expr n "2+2".
Proof.
  unfold parses_toplevel. unfold parses_expr. unfold calc_grammar.
  exists 10.
  right. split.
  2:{ left. exists "2+", "2".
    split.
    * simpl. reflexivity.
    * split.
      { exists "2", "+".
        split.
        - simpl. reflexivity.
        - split.
          * right.
            split.
            { unfold "<>". intro. discriminate. }
            right.
            split.
            { unfold "<>". intro. discriminate. }
            reflexivity.
          * reflexivity.
      } right.
        split.
        { unfold "<>". intro. discriminate. }
        right.
        split.
        { unfold "<>". intro. discriminate. }
        reflexivity.
  }
  unfold "~".
    intro.
    destruct H. destruct H.
    destruct H.
    destruct H0.
    destruct H0. destruct H0.
    destruct H0.
    destruct H2.
    destruct H1.
    - destruct H2.
      * subst. simpl in H. discriminate.
      * destruct H2.
        destruct H4.
        { subst. simpl in H. discriminate. }
        destruct H4.
        subst. simpl in H. discriminate.
    - destruct H1.
      destruct H2.
      * destruct H4.
        { subst. simpl in H. discriminate. }
        destruct H4.
        subst. simpl in H. discriminate.
      * destruct H2.
        destruct H4.
        { destruct H5.
          - subst. simpl in H. discriminate.
          - destruct H5. subst. simpl in H. discriminate.
        } destruct H4.
          destruct H5.
          { subst. simpl in H. discriminate. }
          destruct H5. subst. simpl in H. discriminate.
Qed.

(*
  Expr <- Num '-' Num / Num '+' Num / Num
  Num <- '0' / '1' / '2'
*)

Inductive calc_nt_action : Type -> Type :=
  | calc_num_action: calc_nt_action nat
  | calc_expr_action: calc_nt_action nat
.

Check grammar_action.

Check (1, 2).

Check fun '(x, y) => x + y.

Definition calc_grammar_actions : grammar_action calc_nt_action :=
  fun _ nt =>
  match nt with
  | calc_num_action => ((term "0") $> pure 0) // ((term "1") $> pure 1) // ((term "2") $>  pure 2)
  | calc_expr_action => 
         ((EANonterminal calc_num_action) 
           <$ (term "-")
            $ (EANonterminal calc_num_action)
            @ (fun '(x,y) => x - y))
      // ((EANonterminal calc_num_action) 
           <$ (term "+")
            $ (EANonterminal calc_num_action)
            @ (fun '(x, y) => x + y))
      // (EANonterminal calc_num_action)
end.

Print calc_grammar_actions.

Example action_calc_parses_0_to_0 : exists n,
 parses_expr_action calc_nt_action calc_grammar_actions n "0" (EANonterminal calc_num_action) 0.
Proof.
  exists 10.
  simpl.
  left. left.
  exists "0". exists "".
  split.

    reflexivity.

    exists ("0", 0).
    split.

      exists "0", "".
      split.

        reflexivity.

        exists "0".
        split.

          reflexivity.

          exists "", "".
          split.

            reflexivity.

            exists 0.
            split.

              reflexivity.

              reflexivity.

      reflexivity.
Qed.

Example action_calc_parses_top_0_to_0 : exists n,
  parses_toplevel_action calc_nt_action calc_grammar_actions calc_num_action n "0" 0.
Proof.
  exists 12.
  unfold parses_toplevel_action.
  simpl.
  left. left.
  exists "0", "".
  split.

    reflexivity.

    exists ("0", 0).
    split.

      exists "0", "".
      split.

        reflexivity.

        exists "0".
        split.

          reflexivity.

          exists "", "".
          split.

            reflexivity.

            exists 0.
            split.

              reflexivity.

              reflexivity.

      reflexivity.
Qed.

Lemma pta_alt_right:
  forall NT gram n V (e e': expr_action NT V) s v, 
    parses_expr_action _ gram n s e' v -> 
    ~ parses_expr_action _ gram n s e v -> 
    parses_expr_action _ gram (S n) s (e // e') v.
Admitted.

Lemma pta_alt_fail:
  forall NT gram n V (e e': expr_action NT V) s v, 
    ~ parses_expr_action _ gram n s e v -> 
    ~ parses_expr_action _ gram n s e' v -> 
    ~ parses_expr_action _ gram n s (e // e') v.
Admitted. 

Lemma pta_map_fail:
  forall NT gram n V V' (e: expr_action NT V) s v (f : V -> V'), 
    ~ parses_expr_action _ gram n s e v -> 
    forall v', 
      ~ parses_expr_action _ gram n s (e @ f) v'. 
Admitted.

Lemma pta_unroll_once:
  forall NT gram n V (e: expr_action NT V) s v,
   ( match e in expr_action _ V return V -> Prop with
      | EAReject => fun _ => False
      | EAAccept ea => fun v => ea = v
      | EANonterminal nta => parses_expr_action _ gram n s (gram _ nta)
      | EAAlt e1 e2 => fun v =>
          let e1_output := parses_expr_action _ gram n s e1 v in
          e1_output \/ (~(e1_output) /\ parses_expr_action _ gram n s e2 v)
      | EATerminal s0 => fun _ => s0 = s 
      | EARepeat ear => fun v =>
          exists l,
          s = concat l
          /\ List.length l = List.length v
          /\ (forall plv, In plv (List.combine l v) -> parses_expr_action _ gram n (fst plv) ear (snd plv))
      | EABind ea aeb => fun v =>
          exists s1 s2,
          s = s1 ++ s2 /\
          (exists intermediary,
            (parses_expr_action _ gram n s1 ea intermediary)
            /\
            (parses_expr_action _ gram n s2 (aeb intermediary) v))
      | EANegate ea => fun _ => forall v, ~ (parses_expr_action _ gram n s ea v)
      end v) ->
    parses_expr_action _ gram (S n) s e v.
Proof.
  intros. eauto.
Qed.
    (* match fuel with | 0 => False
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
  end. *)

Example action_calc_parses_start_0_to_0 : exists n,
  parses_toplevel_action calc_nt_action calc_grammar_actions calc_expr_action n "0" 0.
Proof.
  exists 8.
  unfold parses_toplevel_action.
  set (e := (calc_grammar_actions nat calc_expr_action)).
  simpl in e.
  eapply pta_unroll_once.
  subst e.

  cbv iota beta.
  eapply pta_alt_right.
  - admit.
  - eapply pta_alt_fail.
    + eapply pta_map_fail.
      unfold "~".
      intros.
      inversion H.
      destruct H0 as [? [? [? [? ?]]]].
      destruct x.
      * simpl in H0.
        subst.
        inversion H1.
        destruct H0 as [? [? [? [? ?]]]].
        destruct x; try now inversion H0.
        destruct x0; try now inversion H0.
        inversion H3.
        destruct H5 as [? [? [? [? ?]]]].
        destruct x; try now inversion H5.
        destruct x0; try now inversion H5.
        simpl in H7.
        destruct H7 as [? [? [? [? [? ?]]]]].
        destruct x; try now inversion H7.
      * 
      admit.
    + 
Admitted.
(* 
  simpl.
  right.
  split.
  2:{
    left.
    left.
    exists "0", "".
    split.

      reflexivity.

      exists ("0", 0).
      split.

        exists "0", "".
        split.

          reflexivity.

          exists "0".
          split.

            reflexivity.

            exists "", "".
            split.

              reflexivity.

              exists 0.
              split.
              all: reflexivity.
  }

  intro.
  destruct H.

    destruct H. destruct H.
    destruct H.
    destruct H0.
    destruct H0.
    destruct H0. destruct H0.
    destruct H0.
    destruct H2.
    destruct H2.
    destruct H2. destruct H2.
    destruct H2.
    destruct H4.
    destruct H4.
    destruct H4. destruct H4.
    destruct H4.
    destruct H6.
    destruct H6.
    destruct H6.

      assumption.

      destruct H6.
      assumption.

    destruct H. clear H.
    destruct H0. destruct H.
    destruct H.
    destruct H0.
    destruct H0.
    destruct H0. destruct H0.
    destruct H0.
    destruct H2.
    destruct H2.
    destruct H2. destruct H2.
    destruct H2.
    destruct H4.
    destruct H3. destruct H3.
    destruct H3.
    destruct H5.
    destruct H5.
    destruct H5.

      destruct H5.
      assumption.

      destruct H5.
      assumption.

      destruct H5.
      destruct H7. destruct H7.
      destruct H7.
      destruct H8.
      destruct H8.
      assumption.
Qed. *)