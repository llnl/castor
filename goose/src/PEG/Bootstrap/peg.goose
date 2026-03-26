# Syntax of specifying PEGs as in the original paper
Grammar <- Spacing Definition+ EndOfFile
Definition <- Identifier LEFTARROW Expression

Expression <- Sequence (SLASH Sequence)*
Sequence <- Prefix*
Prefix <- (Prefix_Extra)? Suffix
Prefix_Extra <- AND / NOT
Suffix <- Primary (Suffix0)?
Suffix0 <- QUESTION / STAR / PLUS

Primary <- Identifier !LEFTARROW
Primary <- OPEN Expression CLOSE
Primary <- Literal
Primary <- Class
Primary <- DOT

Identifier <- IdentStart IdentCont* Spacing
IdentStart <- [a-zA-Z_]
IdentCont <- IdentStart
         / [0-9]

Literal <- ['] (!['] Char)* ['] Spacing
        /   ["] (!["] Char)* ["] Spacing
Class <- '[' (!'[' Range)* ']' Spacing
Range <- Char '-' Char / Char
Char <- '\\' [nrt'"\[\]\\]
        / '\\' [0-2][0-7][0-7]
        / '\\' [0-7][0-7]?
        / !'\\' .

LEFTARROW <- '<-' Spacing
SLASH <- '/' Spacing
AND <- '&' Spacing
NOT <- '!' Spacing
QUESTION <- '?' Spacing
STAR <- '*' Spacing
PLUS <- "+" Spacing
OPEN <- '(' Spacing
CLOSE <- ')' Spacing
DOT <- '.' Spacing

#Spacing <- (Space / Comment)*
Space <- ' ' / '\t' / EndOfLine
Comment <- '#' (!EndOfLine .)* EndOfLine
EndOfLine <- '\r\n' / '\n' / '\r'
EndOfFile <- !.