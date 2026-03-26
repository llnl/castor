type _ expr = 
  | Term : char -> 'a expr
  (* | Nonterm : 'a -> 'a expr *)
  | Alt : 'a expr * 'a expr -> 'a expr
  | Cat : 'a expr * 'a expr -> 'a expr
  | Star: 'a expr -> 'a expr
  | Neg: 'a expr -> 'a expr

let rec print_expr (e: 'a expr) = 
  begin match e with 
  | Term c -> Printf.sprintf "(Term %c)" c
  | Alt (l, r) -> Printf.sprintf "(Alt %s %s)" (print_expr l) (print_expr r)
  | Cat (l, r) -> Printf.sprintf "(Cat %s %s)" (print_expr l) (print_expr r)
  | Star e -> Printf.sprintf "(Star %s)" (print_expr e)
  | Neg e -> Printf.sprintf "(Neg %s)" (print_expr e)
  end

let rec parse (e: 'a expr) (cs: char list) : (char list) option = 
  begin match e with 
  | Term c -> 
    begin match cs with 
    | c' :: cs' -> 
      if c = c' then Some cs' else None
    | [] -> None
    end
  | Alt (l, r) -> 
    begin match parse l cs with 
    | Some cs' -> parse r cs'
    | None -> parse r cs
    end
  | Cat (l, r) -> 
    begin match parse l cs with 
    | Some cs' -> parse r cs'
    | None -> None
    end
  | Star e -> 
    begin match parse e cs with 
    | Some cs' -> parse (Star e) cs'
    | None -> Some cs
    end
  | Neg e -> 
    begin match parse e cs with 
    | Some _ -> None
    | None -> Some cs
    end
  end

let explode_string s = List.init (String.length s) (String.get s);;

let implode_string (cs : char list) : string = String.of_seq (List.to_seq cs)


let rec prefixes_aux cs = 
  match cs with 
  | [] -> [[]]
  | _ :: cs' -> 
    cs :: prefixes_aux cs'
let prefixes cs = 
  prefixes_aux cs

let parse_str e s = 
  begin match parse e (explode_string s) with 
  | Some [] -> true
  | _ -> false
  end

type memo_entry = {
  num_parsed : int
}

let rec list_prefix n xs = 
  if n > 0 then 
    match xs with 
    | [] -> ([], [])
    | x :: xs' -> 
      let (p, s) = list_prefix (n-1) xs' in (x :: p, s)
  else ([], xs)
  
type 'a memo_table = ('a expr * string, memo_entry) Hashtbl.t

let prefix_char s c = String.make 1 c ^ s
let suffix_char s c = s ^ String.make 1 c

let rec parse_memo ~(tbl : 'a memo_table) e prefix suffix = 
  Printf.printf "parsing %s on \"%s\" with prefix \"%s\"\n" (print_expr e) (implode_string suffix) prefix ;
  let mem_result = List.fold_left (fun acc x -> 
    match acc with 
    | Some _ -> acc
    | None -> Hashtbl.find_opt tbl (e, implode_string x)
  ) None (prefixes suffix) in 
  let strs = List.map implode_string (prefixes suffix) in 
  Printf.printf "prefixes: {%s}\n" (String.concat "," strs) ;
  let entries = List.map (fun ((e, s), v) -> Format.sprintf "(%s, %s) |-> %n" (print_expr e) s v.num_parsed) (List.of_seq (Hashtbl.to_seq tbl)) in 
  Printf.printf "tbl: {%s}\n" (String.concat "," entries);
  match mem_result with 
  | Some r -> 
    
    let (prefix_ls, suffix) = list_prefix r.num_parsed suffix in 
      Printf.printf ("using the memo table!! skipping %d of %s \n") r.num_parsed (implode_string suffix) ;
      Some (prefix ^ implode_string prefix_ls, suffix)
  | None ->
    begin match e with 
    | Term c -> 
      begin match suffix with 
      | c' :: cs' -> 
        if c = c' then 
          begin
            Printf.printf "Success! taking one\n" ;
            Hashtbl.add tbl (e, String.of_seq (List.to_seq [c])) {num_parsed = 1} ;
            Some (suffix_char prefix c, cs') 
          end else None
      | [] -> None
      end
    | Alt (l, r) -> 
      begin match parse_memo ~tbl l prefix suffix with 
      | Some (prefix, suffix) -> Some (prefix, suffix)
      | None -> parse_memo ~tbl r prefix suffix
      end
    | Cat (l, r) -> 
      begin match parse_memo ~tbl l prefix suffix with 
      | Some (prefix', suffix') -> parse_memo ~tbl r prefix' suffix'
      | None -> None
      end
    | Star e -> 
      begin match parse_memo ~tbl e prefix suffix with 
      | Some (prefix', suffix') -> 
        Printf.printf "continuing star loop \n"; 
        parse_memo ~tbl (Star e) prefix' suffix'
      | None -> 
        Printf.printf "done with star loop, prefix is \"%s\", len is %d\n" prefix (String.length prefix); 
        Hashtbl.add tbl (Star e, prefix) {num_parsed = String.length prefix} ;
        Some (prefix, suffix)
      end
    | Neg e -> 
      begin match parse_memo ~tbl e prefix suffix with 
      | Some _ -> None
      | None -> Some (prefix, suffix)
      end
    end
    

let parse_str_memo ~(tbl: 'a memo_table) e s = 
  begin match parse_memo ~tbl e "" (explode_string s) with 
  | Some (_, []) -> true
  | _ -> false
  end