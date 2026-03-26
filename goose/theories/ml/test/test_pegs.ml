open Pegs.Peg

let t1 : unit expr = Star (Term 'a')

let s1 = "aa" ;;
let s2 = "aaa" ;;



Printf.printf "output for t1: %b\n" (parse_str t1 s1) ;;
Printf.printf "output for t2: %b\n" (parse_str t1 s2) ;;

let tbl = Hashtbl.create 10 ;;

Printf.printf "output for t1 (memo): %b\n" (parse_str_memo ~tbl t1 s1) ;;
Printf.printf "output for t2 (memo): %b\n" (parse_str_memo ~tbl t1 s2) ;;

