type _ expr = 
  | Term : char -> 'a expr
  (* | Nonterm : 'a -> 'a expr *)
  | Alt : 'a expr * 'a expr -> 'a expr
  | Cat : 'a expr * 'a expr -> 'a expr
  | Star: 'a expr -> 'a expr
  | Neg: 'a expr -> 'a expr

val print_expr : 'a expr -> string

val parse : 'a expr -> char list -> (char list) option

val parse_str : 'a expr -> string -> bool

type memo_entry = {
  num_parsed : int
}

type 'a memo_table = ('a expr * string, memo_entry) Hashtbl.t

val parse_memo : tbl: 'a memo_table -> 'a expr -> string -> char list -> (string * char list) option

val parse_str_memo : tbl: 'a memo_table -> 'a expr -> string -> bool