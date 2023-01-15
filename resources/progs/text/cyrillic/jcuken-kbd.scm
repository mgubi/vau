
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; MODULE      : jcuken-kbd.scm
;; DESCRIPTION : typing russian using the jcuken keyboard encoding
;;
;; This software falls under the GNU general public license version 3 or later.
;; It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
;; in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(texmacs-module (text cyrillic jcuken-kbd)
  (:use (text text-kbd)))

(kbd-map
  (:mode in-cyrillic-jcuken?)

  ("q" "<#439>")
  ("w" "<#446>")
  ("e" "<#443>")
  ("r" "<#43A>")
  ("t" "<#435>")
  ("y" "<#43D>")
  ("u" "<#433>")
  ("i" "<#448>")
  ("o" "<#449>")
  ("p" "<#437>")
  ("[" "<#445>")
  ("]" "<#44A>")
  ("a" "<#444>")
  ("s" "<#44B>")
  ("d" "<#432>")
  ("f" "<#430>")
  ("g" "<#43F>")
  ("h" "<#440>")
  ("j" "<#43E>")
  ("k" "<#43B>")
  ("l" "<#434>")
  (";" "<#436>")
  ("'" "<#44D>")
  ("z" "<#44F>")
  ("x" "<#447>")
  ("c" "<#441>")
  ("v" "<#43C>")
  ("b" "<#438>")
  ("n" "<#442>")
  ("m" "<#44C>")
  ("," "<#431>")
  ("." "<#44E>")
  ("`" "<#451>")

  ("Q" "<#419>")
  ("W" "<#426>")
  ("E" "<#423>")
  ("R" "<#41A>")
  ("T" "<#415>")
  ("Y" "<#41D>")
  ("U" "<#413>")
  ("I" "<#428>")
  ("O" "<#429>")
  ("P" "<#417>")
  ("{" "<#425>")
  ("}" "<#42A>")
  ("A" "<#424>")
  ("S" "<#42B>")
  ("D" "<#412>")
  ("F" "<#410>")
  ("G" "<#41F>")
  ("H" "<#420>")
  ("J" "<#41E>")
  ("K" "<#41B>")
  ("L" "<#414>")
  (":" "<#416>")
  ("\"" "<#42D>")
  ("Z" "<#42F>")
  ("X" "<#427>")
  ("C" "<#421>")
  ("V" "<#41C>")
  ("B" "<#418>")
  ("N" "<#422>")
  ("M" "<#42C>")
  ("<" "<#411>")
  (">" "<#42E>")
  ("~" "<#401>")

  ("@" "\"")
  ("#" "'")
  ("$" "*")
  ("%" ":")
  ("^" ",")
  ("&" ".")
  ("*" ";")

  ("accent:umlaut t" "<#451>")
  ("accent:umlaut T" "<#401>"))
