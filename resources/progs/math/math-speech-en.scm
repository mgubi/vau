
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; MODULE      : math-speech-en.scm
;; DESCRIPTION : mathematical editing using English speech
;; COPYRIGHT   : (C) 2022  Joris van der Hoeven
;;
;; This software falls under the GNU general public license version 3 or later.
;; It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
;; in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(texmacs-module (math math-speech-en)
  (:use (math math-speech)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Sanitize input
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-table english-numbers
  ("0" "zero") ("1" "one") ("2" "two") ("3" "three") ("4" "four")
  ("5" "five") ("6" "six") ("7" "seven") ("8" "eight") ("9" "nine"))

(define-table english-ambiguate
  ("d" "d/b/p") ("m" "m/n") ("s" "s/f"))

(define (string-table-replace s t)
  (with repl (lambda (x) (with y (ahash-ref t x) (if y (car y) x)))
    (with l (string-decompose s " ")
      (string-recompose (map repl l) " "))))

(define (rewrite-/ s)
  (with l (string-decompose s "/")
    (if (and (== (length l) 2)
             (string-number? (car l))
             (string-number? (cadr l)))
        (string-replace s "/" " @over ")
        s)))

(tm-define (speech-sanitize lan mode s)
  (:require (and (== lan 'english) (== mode 'math)))
  (set! s (locase-all s))
  (set! s (letterize s))
  (set! s (list->tmstring (clean-letter-digit (tmstring->list s))))
  (set! s (string-recompose (map rewrite-/ (string-decompose s " ")) " "))
  (set! s (string-replace s "+" " plus "))
  (set! s (string-replace-trailing s "-" " minus "))
  (set! s (string-replace s "<times>" " times "))
  (set! s (string-replace s "/" " slash "))
  (set! s (string-replace s "," " comma "))
  (set! s (string-replace-trailing s "." " period "))
  (set! s (string-replace s ":" " colon "))
  (set! s (string-replace s ";" " semicolon "))
  (set! s (string-replace s "^" " hat "))
  (set! s (string-replace s "~" " tilda "))
  (set! s (string-replace s "(" " ( "))
  (set! s (string-replace s ")" " ) "))
  (set! s (string-replace s "[" " [ "))
  (set! s (string-replace s "]" " ] "))
  (set! s (string-replace s "{" " { "))
  (set! s (string-replace s "}" " } "))
  (set! s (string-replace s "<ldots>" " dots "))
  (set! s (string-replace s "<cdots>" " dots "))
  (set! s (string-table-replace s english-numbers))
  (set! s (string-table-replace s english-ambiguate))
  (set! s (string-replace s "  " " "))
  (set! s (string-replace s "  " " "))
  (set! s (tm-string-trim-both s))
  s)

(speech-collection dont-break english
  "ad" "ag" "ah" "al" "an" "ar" "as" "eg" "el" "em" "en" "ex"
  "if" "in" "is" "it" "of" "oh" "ok" "ol" "or" "up"
  "be" "de" "he" "pe" "se" "ve" "we"
  "ma" "va" "bi" "hi" "ji" "pi" "si" "xi" "yi"
  "do" "fo" "ho" "jo" "ko" "lo" "no" "po" "so" "to" "vo" "wo"
  "mu" "nu" "by" "hy" "ky" "my" "sy")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Tables for recognizing mathematics inside text
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(speech-collection prefix english
  "big" "small" "capital" "uppercase" "lowercase"
  "bold" "upright" "calligraphic" "fraktur" "gothic"
  "blackboard bold" "sans serif" "typewriter")

(speech-collection prefix english
  "exponential" "logarithm" "sine" "cosine" "tangent"
  "square root")

(speech-collection postfix english
  "prime" "dagger" "square" "squared" "cube" "cubed")

(speech-collection infix english
  "equal" "assign" "congruent"
  "superior" "inferior" "smaller" "larger" "less" "greater")

(speech-collection math-mode english
  "math" "maths" "mathematics")

(speech-collection text-mode english
  "text")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Entering mathematical symbols via English speech
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(speech-symbols english
  ("zero" "0")
  ("one" "1")
  ("two" "2")
  ("three" "3")
  ("four" "4")
  ("five" "5")
  ("six" "6")
  ("seven" "7")
  ("eight" "8")
  ("nine" "9")
  ("ten" "10")
  ("hundred" "100")
  ("thousand" "1000")
  ("million" "1000000")
  ("billion" "1000000000")

  ("a" "a")
  ("b" "b")
  ("c" "c")
  ("d" "d")
  ("e" "e")
  ("f" "f")
  ("g" "g")
  ("h" "h")
  ("i" "i")
  ("j" "j")
  ("k" "k")
  ("l" "l")
  ("m" "m")
  ("n" "n")
  ("o" "o")
  ("p" "p")
  ("q" "q")
  ("r" "r")
  ("s" "s")
  ("t" "t")
  ("u" "u")
  ("v" "v")
  ("w" "w")
  ("x" "x")
  ("y" "y")
  ("z" "z")

  ("alpha" "<alpha>")
  ("beta" "<beta>")
  ("gamma" "<gamma>")
  ("delta" "<delta>")
  ("epsilon" "<epsilon>")
  ("zeta" "<zeta>")
  ("eta" "<eta>")
  ("theta" "<theta>")
  ("iota" "<iota>")
  ("kappa" "<kappa>")
  ("lambda" "<lambda>")
  ("mu" "<mu>")
  ("nu" "<nu>")
  ("xi" "<xi>")
  ("omicron" "<omicron>")
  ("pi" "<pi>")
  ("rho" "<rho>")
  ("sigma" "<sigma>")
  ("tau" "<tau>")
  ("upsilon" "<upsilon>")
  ("phi" "<phi>")
  ("psi" "<psi>")
  ("chi" "<chi>")
  ("omega" "<omega>")

  ("constant e" "<mathe>")
  ("constant i" "<mathi>")
  ("constant pi" "<mathpi>")
  ("constant gamma" "<mathgamma>")
  ("euler constant" "<mathgamma>")

  ("infinity" "<infty>")
  ("complex numbers" "<bbb-C>")
  ("positive integers" "<bbb-N>")
  ("rationals" "<bbb-Q>")
  ("reals" "<bbb-R>")
  ("integers" "<bbb-Z>")

  ("plus" "+")
  ("minus" "-")
  ("times" "*")
  ("cross" "<times>")
  ("slash" "/")
  ("apply" " ")
  ("space" " ")
  ("after" "<circ>")
  ("tensor" "<otimes>")
  ("factorial" "!")

  ("equal" "=")
  ("not equal" "<neq>")
  ("assign" "<assign>")
  ("defined as" "<assign>")
  ("congruent" "<equiv>")
  ("less" "<less>")
  ("less equal" "<leqslant>")
  ("greater" "<gtr>")
  ("greater equal" "<geqslant>")
  ("much less" "<ll>")
  ("much greater" "<gg>")

  ("element" "<in>")
  ("is in" "<in>")
  ("not in" "<nin>")
  ("is not in" "<nin>")
  ("contains" "<ni>")
  ("subset" "<subset>")
  ("superset" "<supset>")
  ("intersection" "<cap>")
  ("union" "<cup>")

  ("similar" "<sim>")
  ("asymptotic" "<asymp>")
  ("approx" "<approx>")
  ("isomorphic" "<cong>")
  ("negligible" "<prec>")
  ("dominated" "<preccurlyeq>")
  ("dominates" "<succcurlyeq>")
  ("strictly dominates" "<succ>")

  ("for all" "<forall>")
  ("exists" "<exists>")
  ("or" "<vee>")
  ("logical and" "<wedge>")
  ("implies" "<Rightarrow>")
  ("equivalent" "<Leftrightarrow>")

  ("right arrow" "<rightarrow>")
  ("long right arrow" "<rightarrow>")
  ("maps to" "<mapsto>")
  ("long maps to" "<longmapsto>")

  ("period" ".")
  ("comma" ",")
  ("colon" ":")
  ("semicolon" ";")
  ("exclamation mark" "!")
  ("question mark" "?")
  ("." ".")
  ("," ",")
  (":" ":")
  (";" ";")
  ("!" "!")
  ("?" "?")
  ("such that" "<suchthat>")
  )

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; More complex mathematical speech commands
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(speech-map english math
  ("uppercase" (speech-alter-letter :big))
  ("lowercase" (speech-alter-letter :small))
  ("bold" (speech-alter-letter :bold))
  ("medium" (speech-alter-letter :medium))
  ("upright" (speech-alter-letter :up))
  ("italic" (speech-alter-letter :it))
  ("calligraphic" (speech-alter-letter :cal))
  ("fraktur" (speech-alter-letter :frak))
  ("gothic" (speech-alter-letter :frak))
  ("blackboard bold" (speech-alter-letter :bbb))
  ("double" (speech-alter-letter :bbb))
  ("normal" (speech-alter-letter :normal))
  ("sans serif" (speech-alter-letter :ss))
  ("typewriter" (speech-alter-letter :tt))
  ("operator" (speech-operator))
 
  ("factor" (speech-factor))
  ("inverse" (speech-insert-superscript "-1"))
  ("square" (speech-insert-superscript "2"))
  ("cube" (speech-insert-superscript "3"))
  ("transpose" (speech-insert-superscript "<top>"))
  ("sub" (speech-short-subscript))
  ("power" (speech-short-superscript))
  ("subscript" (speech-subscript))
  ("superscript" (speech-superscript))
  ("start subscript" (make 'rsub))
  ("start superscript" (make 'rsup))
  ("end subscript" (speech-end 'rsub))
  ("end superscript" (speech-end 'rsup))

  ("prime" (make-rprime "'"))
  ("double prime" (make-rprime "'") (make-rprime "'"))
  ("triple prime" (make-rprime "'") (make-rprime "'") (make-rprime "'"))
  ("dagger" (make-rprime "<dag>"))
  ("adjoint" (make-rprime "<asterisk>"))

  ("hat" (speech-accent "^"))
  ("tilda" (speech-accent "~"))
  ("bar" (speech-accent "<bar>"))
  ("wide hat" (speech-wide "^"))
  ("wide tilda" (speech-wide "~"))
  ("wide bar" (speech-wide "<bar>"))
  ("under" (speech-under))

  ("of" (speech-of))
  ("open" (speech-open "(" ")"))
  ("close" (speech-close))
  ("parentheses" (speech-brackets "(" ")"))
  ("brackets" (speech-brackets "[" "]"))
  ("braces" (speech-brackets "{" "}"))
  ("chevrons" (speech-brackets "<langle>" "<rangle>"))
  ("floor" (speech-brackets "<lfloor>" "<rfloor>"))
  ("ceiling" (speech-brackets "<lceil>" "<rceil>"))
  ("open parentheses" (speech-open "(" ")"))
  ("open open" (speech-open "[" "]"))
  ("open braces" (speech-open "{" "}"))
  ("open chevrons" (speech-open "<langle>" "<rangle>"))
  ("open floor" (speech-open "<lfloor>" "<rfloor>"))
  ("open ceiling" (speech-open "<lceil>" "<rceil>"))

  ("(" (speech-open "(" ")"))
  ("[" (speech-open "[" "]"))
  ("{" (speech-open "{" "}"))
  (")" (speech-close))
  ("]" (speech-close))
  ("}" (speech-close))
  
  ("arc cos" (speech-insert-operator "arccos"))
  ("arc sin" (speech-insert-operator "arcsin"))
  ("arc tan" (speech-insert-operator "arctan"))
  ("arg" (speech-insert-operator "arg"))
  ("cos" (speech-insert-operator "cos"))
  ("deg" (speech-insert-operator "deg"))
  ("det" (speech-insert-operator "det"))
  ("dim" (speech-insert-operator "dim"))
  ("exp" (speech-insert-operator "exp"))
  ("gcd" (speech-insert-operator "gcd"))
  ("log" (speech-insert-operator "log"))
  ("hom" (speech-insert-operator "hom"))
  ("inf" (speech-insert-operator "inf"))
  ("ker" (speech-insert-operator "ker"))
  ("lcm" (speech-insert-operator "lcm"))
  ("lim" (speech-insert-operator "lim"))
  ("lim inf" (speech-insert-operator "liminf"))
  ("lim sup" (speech-insert-operator "limsup"))
  ("ln" (speech-insert-operator "ln"))
  ("log" (speech-insert-operator "log"))
  ("max" (speech-insert-operator "max"))
  ("min" (speech-insert-operator "min"))
  ("Pr" (speech-insert-operator "Pr"))
  ("sin" (speech-insert-operator "sin"))
  ("supremum" (speech-insert-operator "sup"))
  ("tan" (speech-insert-operator "tan"))

  ("mod" (speech-insert-infix-operator "mod"))
  ("div" (speech-insert-infix-operator "div"))
  ("quo" (speech-insert-infix-operator "quo"))
  ("rem" (speech-insert-infix-operator "rem"))
  ("division" (speech-insert-infix-operator "div"))
  ("modulo" (speech-insert-infix-operator "mod"))
  ("quotient" (speech-insert-infix-operator "quo"))
  ("remainder" (speech-insert-infix-operator "rem"))
  ("pseudo remainder" (speech-insert-infix-operator "prem"))

  ("plus dots plus" (speech-dots "+" "<cdots>"))
  ("minus dots minus" (speech-dots "-" "<cdots>"))
  ("times dots times" (speech-dots "*" "<cdots>"))
  ("comma dots comma" (speech-dots "," "<ldots>"))
  ("colon dots colon" (speech-dots ":" "<ldots>"))
  ("semicolon dots semicolon" (speech-dots ";" "<ldots>"))
  ("and dots and" (speech-dots "<wedge>" "<cdots>"))
  ("or dots or" (speech-dots "<vee>" "<cdots>"))
  ("equal dots equal" (speech-dots "=" "<cdots>"))
  ("similar dots similar" (speech-dots "<sim>" "<cdots>"))
  ("less dots less" (speech-dots "<less>" "<cdots>"))
  ("less equal dots less equal" (speech-dots "<leqslant>" "<cdots>"))
  ("greater dots greater" (speech-dots "<gtr>" "<cdots>"))
  ("greater equal dots greater equal" (speech-dots "<geqslant>" "<cdots>"))
  ("tensor dots tensor" (speech-dots "<otimes>" "<cdots>"))

  ("sum" (speech-big-operator "sum"))
  ("product" (speech-big-operator "prod"))
  ("tensor product" (speech-big-operator "otimes"))
  ("integral" (speech-big-operator "int"))
  ("contour integral" (speech-big-operator "oint"))
  ("double integral" (speech-big-operator "iint"))
  ("triple integral" (speech-big-operator "iiint"))
  ("from" (speech-for))
  ("until" (speech-until))

  ("square root" (speech-sqrt-of))
  ("square root of" (speech-sqrt-of))
  ("start square root" (speech-sqrt))
  ("end square root" (speech-end 'sqrt))
  ("over" (speech-over))
  ("@over" (speech-short-over))
  ("fraction" (speech-fraction))
  ("start fraction" (speech-fraction))
  ("numerator" (go-to-fraction :numerator))
  ("denominator" (go-to-fraction :denominator))
  ("end fraction" (speech-end 'frac))

  ("matrix" (make 'matrix))
  ("determinant" (make 'det))
  ("choice" (make 'choice))
  ("horizontal dots" (insert "<cdots>"))
  ("vertical dots" (insert "<vdots>"))
  ("diagonal dots" (insert "<ddots>"))
  ("upward dots" (insert "<udots>"))

  ;;("more" "var")
  )

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Commonly used unambiguous words for letters
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(speech-reduce english math
  ("alfred" "a")
  ("benjamin" "b")
  ("benji" "b")
  ("charles" "c")
  ("charlie" "c")
  ("david" "d")
  ("eddie" "e")
  ("edward" "e")
  ("frederick" "f")
  ("george" "g")
  ("harry" "h")
  ("isaac" "i")
  ("isaak" "i")
  ("jack" "j")
  ("king" "k")
  ("london" "l")
  ("mary" "m")
  ("nellie" "n")
  ("nelly" "n")
  ("oliver" "o")
  ("peter" "p")
  ("queen" "q")
  ("robert" "r")
  ("samuel" "s")
  ("tommy" "t")
  ("uncle" "u")
  ("victor" "v")
  ("william" "w")
  ("x-ray" "x")
  ("yellow" "y")
  ("zebra" "z"))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Speech reductions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(speech-reduce english math
  ("digit zero" "zero")
  ("digit one" "one")
  ("digit two" "two")
  ("digit three" "three")
  ("digit d/t/v/3" "three")
  ("digit four" "four")
  ("digit five" "five")
  ("digit phi/5" "five")
  ("digit 5/phi" "five")
  ("digit six" "six")
  ("digit seven" "seven")
  ("digit eight" "eight")
  ("digit a/e/8" "eight")
  ("digit nine" "nine")
  
  ("letter a" "a")
  ("letter b" "b")
  ("letter c" "c")
  ("letter d" "d")
  ("letter d/b" "d")
  ("letter d/v" "d")
  ("letter d/b/p" "d")
  ("letter d/t/v/3" "d")
  ("letter e" "e")
  ("letter f" "f")
  ("letter g" "g")
  ("letter h" "h")
  ("letter i" "i")
  ("letter j" "j")
  ("letter k" "k")
  ("letter l" "l")
  ("letter m" "m")
  ("letter m/n" "m")
  ("letter n" "n")
  ("letter o" "o")
  ("letter p" "p")
  ("letter q" "q")
  ("letter r" "r")
  ("letter s" "s")
  ("letter s/f" "s")
  ("letter t" "t")
  ("letter u" "u")
  ("letter v" "v")
  ("letter w" "w")
  ("letter x" "x")
  ("letter y" "y")
  ("letter z" "z")

  ("greek phi" "phi")

  ("big" "uppercase")
  ("capital" "uppercase")
  ("small" "lowercase")

  ("the complex" "complex")
  ("the positive integers" "positive integers")
  ("the rationals" "rationals")
  ("the reals" "reals")
  ("the integers" "integers")
  ("rational" "rationals")
  ("rational numbers" "rationals")
  ("real" "reals")
  ("real numbers" "reals")
  ("double stroke" "blackboard bold")

  ("parenthesis" "parentheses")
  ("bracket" "brackets")
  ("brace" "braces")
  ("chevron" "chevrons")
  ("round brackets" "parentheses")
  ("square brackets" "brackets")
  ("curly brackets" "braces")
  ("angular brackets" "chevrons")
  ("close parentheses" "close")
  ("close brackets" "close")
  ("close braces" "close")
  ("close chevrons" "close")
  ("close floor" "close")
  ("close ceiling" "close")

  ("set" "braces")
  ("set of" "braces")
  ("the set" "braces")
  ("the set of" "braces")

  ("begin subscript" "start subscript")
  ("begin superscript" "start superscript")
  ("begin square root" "start square root")
  ("begin fraction" "start fraction")
  ("insert subscript" "start subscript")
  ("insert superscript" "start superscript")
  ("insert square root" "start square root")
  ("insert fraction" "start fraction")
  ("and subscript" "end subscript")
  ("and superscript" "end superscript")
  ("and square root" "end square root")
  ("and fraction" "end fraction")

  ("to the" "power")
  ("to the power" "power")
  ("exponent" "superscript")
  ("squared" "square")
  ("cubed" "cube")
  ("transposed" "transpose")

  ("equals" "equal")
  ("equal to" "equal")
  ("is equal to" "equal")

  ("not equal to" "not equal")
  ("is not equal" "not equal")
  ("is not equal to" "not equal")
  ("unequal" "not equal")
  ("unequal to" "not equal")
  ("is unequal to" "not equal")
  ("different" "not equal")
  ("different from" "not equal")
  ("is different from" "not equal")

  ("smaller" "less")
  ("bigger" "greater")
  ("larger" "greater")
  ("less then" "less than")
  ("greater then" "greater than")
  ("difference" "different")

  ("inferior" "less")
  ("inferior to" "inferior")
  ("very inferior" "much less")
  ("is less" "less")
  ("less than" "less")
  ("less or equal" "less equal")
  ("is much less" "much less")

  ("superior" "greater")
  ("superior to" "superior")
  ("very superior" "much greater")
  ("is greater" "greater")
  ("greater than" "greater")
  ("greater or equal to" "greater equal")
  ("is much greater" "much greater")

  ("belongs" "element")
  ("belongs to" "element")
  ("member" "element")
  ("member of" "element")
  ("is a member of" "element")
  ("element of" "element")
  ("is an element of" "element")
  ("does not belong to" "not in")
  ("is not a member of" "not in")
  ("is not an element of" "not in")

  ("subset of" "subset")
  ("is a subset of" "subset")
  ("superset of" "superset")
  ("is a superset of" "superset")
  ("into" "right arrow")
  
  ("there exists a" "exists")
  ("there exists an" "exists")
  ("there exists" "exists")
  ("if and only if" "equivalent")

  ("argument" "arg")
  ("cosine" "cos")
  ("degree" "deg")
  ("dimension" "dim")
  ("exponential" "exp")
  ("greatest common divisor" "gcd")
  ("homomorphisms" "hom")
  ("infimum" "inf")
  ("kernel" "ker")
  ("least common multiple" "lcm")
  ("limit" "lim")
  ("inferior lim" "lim inf")
  ("superior sup" "lim sup")
  ("natural logarithm" "ln")
  ("logarithm" "log")
  ("maximum" "max")
  ("minimum" "min")
  ("probability" "Pr")
  ("sine" "sin")
  ("tangent" "tan")

  ("the arg" "arg")
  ("the cos" "cos")
  ("the deg" "deg")
  ("the det" "det")
  ("the dim" "dim")
  ("the exp" "exp")
  ("the gcd" "gcd")
  ("the hom" "hom")
  ("the inf" "inf")
  ("the ker" "ker")
  ("the lcm" "lcm")
  ("the lim" "lim")
  ("the lim inf" "lim inf")
  ("the lim sup" "lim sup")
  ("the ln" "ln")
  ("the log" "log")
  ("the max" "max")
  ("the min" "min")
  ("the Pr" "Pr")
  ("the sin" "sin")
  ("the supremum" "supremum")
  ("the tan" "tan")
  ("the square root" "square root")

  ("etc." "dots")
  ("etcetera" "dots")
  ("little dots" "dots")
  ("three little dots" "dots")
  ("dot dot dot" "dots")
  ("plus plus" "plus dots plus")
  ("times times" "times dots times")
  ("comma comma" "comma dots comma")
  ("colon colon" "colon dots colon")
  ("semicolon semicolon" "semicolon dots semicolon")
  ("tensor tensor" "tensor dots tensor")
  ("plus until" "plus dots plus")
  ("times until" "times dots times")
  ("comma until" "comma dots comma")
  ("colon until" "colon dots colon")
  ("semicolon until" "semicolon dots semicolon")
  ("and until" "and dots and")
  ("or until" "or dots or")
  ("equal until" "equal dots equal")
  ("similar until" "similar dots similar")
  ("less until" "less dots less")
  ("less equal until" "less equal dots less equal")
  ("greater until" "greater dots greater")
  ("greater equal until" "greater equal dots greater equal")
  ("tensor until" "tensor dots tensor")

  ("similar to" "similar")
  ("is similar to" "similar")
  ("equivalent" "similar")
  ("equivalent to" "equivalent")
  ("is equivalent to" "equivalent")
  ("asymptotic to" "asymptotic")
  ("is asymptotic to" "asymptotic")
  ("approximately" "approx")
  ("approximately equal" "approx")
  ("approximately equal to" "approx")
  ("is approximately" "approximately")
  ("isomorphic to" "isomorphic")
  ("is isomorphic to" "isomorphic")
  
  ("negligible with respect to" "negligible")
  ("is negligible with respect to" "negiglible")
  ("is strictly dominated by" "negligible")
  ("dominated by" "dominated")
  ("is dominated by" "dominated")
  ("dominates" "dominates")

  ("tilde" "tilda")
  ("big hat" "wide hat")
  ("big tilda" "wide tilda")
  ("big tilde" "wide tilda")
  ("big bar" "wide bar")

  ("big sum" "sum")
  ("big product" "product")
  ("big tensor product" "tensor product")
  ("big integral" "integral")
  )
