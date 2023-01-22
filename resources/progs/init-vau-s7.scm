
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; MODULE      : init-vau-s7.scm
;; DESCRIPTION : This is the Vau initialization file (S7)
;; COPYRIGHT   : (C) 2022  Joris van der Hoeven & Massimiliano Gubinelli
;;
;; This software falls under the GNU general public license version 3 or later.
;; It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
;; in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;; S7 macros are not usual macros...
(define define-macro define-expansion)

(define primitive-symbol? symbol?)
(set! symbol? (lambda (s) (and (not (keyword? s)) (primitive-symbol? s))))

;; S7 loads by default in rootlet and eval in curlet
;; but we prefer to load and eval into *texmacs-user-module*
;; (the current toplevel)
;; FIXME: we have to clarify the situation with *current-module* when evaluating
;; in a different environment. In Guile *current-module* is set/reset.

(varlet (rootlet) '*current-module* (curlet))
(let ()
  (define primitive-load load)
  (define primitive-eval eval)
  (define primitive-catch catch)
  
  (varlet (rootlet) 'tm-eval (lambda (obj) (eval obj *texmacs-user-module*)))
  (set! load (lambda (file . env) (primitive-load file (if (null? env) *current-module* (car env)))))
  (set! eval (lambda (obj . env)
    (let ((res (primitive-eval obj (if (null? env) *current-module* (car env)))))
    ;;(format #t "Eval: ~A -> ~A\n" obj res)
    res)
    ))
    
  (set! catch (lambda ( key cl hdl )
    (primitive-catch key cl
      (lambda args
        (apply hdl (car args) "[not-implemented]" (caadr args)  (list (cdadr args)))))))
  )


(let ()
  (display "Benchmark 1\n")
  (define start (texmacs-time))
  (define (fib n) (if (< n 2) n (+ (fib (- n 1)) (fib (- n 2)))))
  (display (fib 30))
  (newline)
  (display "Time: ") (display (- (texmacs-time) start)) (newline)
)

(define developer-mode? #f)
(define boot-start (texmacs-time))
(define remote-client-list (list))

(display "Booting Vau kernel functionality\n")
(load (url-concretize "$TEXMACS_PATH/progs/kernel/boot/boot-s7.scm"))

(inherit-modules (kernel boot compat-s7) (kernel boot abbrevs)
                 (kernel boot debug) (kernel boot srfi)
                 (kernel boot ahash-table) (kernel boot prologue))
(inherit-modules (kernel library base) (kernel library list)
                 (kernel library tree) (kernel library content)
                 (kernel library patch))
(inherit-modules (kernel regexp regexp-match) (kernel regexp regexp-select))
(inherit-modules (kernel logic logic-rules) (kernel logic logic-query)
                 (kernel logic logic-data))
(inherit-modules (kernel texmacs tm-define)
                 (kernel texmacs tm-preferences) (kernel texmacs tm-modes)
                 (kernel texmacs tm-plugins) (kernel texmacs tm-secure)
                 (kernel texmacs tm-convert) (kernel texmacs tm-dialogue)
                 (kernel texmacs tm-language) (kernel texmacs tm-file-system)
                 (kernel texmacs tm-states))
;(inherit-modules (kernel gui gui-markup)
;                 (kernel gui menu-define) (kernel gui menu-widget)
;                 (kernel gui kbd-define) (kernel gui kbd-handlers)
;                 (kernel gui menu-test)
;                 (kernel old-gui old-gui-widget)
;                 (kernel old-gui old-gui-factory)
;                 (kernel old-gui old-gui-form)
;                 (kernel old-gui old-gui-test))
;;(display* "time: " (- (texmacs-time) boot-start) "\n")
;;(display* "memory: " (texmacs-memory) " bytes\n")


(display "Booting converters\n")
(lazy-format (convert rewrite init-rewrite) texmacs verbatim)
(lazy-format (convert tmml init-tmml) tmml)
;(lazy-format (convert latex init-latex) latex)
;(lazy-format (convert html init-html) html)
;(lazy-format (convert bibtex init-bibtex) bibtex)
;(lazy-format (convert images init-images)
;             postscript pdf xfig xmgrace svg xpm jpeg ppm gif png pnm)
;(lazy-define (convert images tmimage)
;             export-selection-as-graphics clipboard-copy-image)
;(lazy-define (convert rewrite init-rewrite) texmacs->code texmacs->verbatim)
;(lazy-define (convert html tmhtml) ext-tmhtml-eqnarray*)
;(define-secure-symbols ext-tmhtml-eqnarray*)
;(lazy-define (convert html tmhtml-expand) tmhtml-env-patch)
;(lazy-define (convert latex latex-drd) latex-arity latex-type)
;(lazy-define (convert latex tmtex) tmtex-env-patch)
;(lazy-define (convert latex latex-tools) latex-set-virtual-packages
;             latex-has-style? latex-has-package?
;             latex-has-texmacs-style? latex-has-texmacs-package?)
;(lazy-menu (convert latex tmtex-widgets) tmtex-menu)
;(display* "time: " (- (texmacs-time) boot-start) "\n")
;(display* "memory: " (texmacs-memory) " bytes\n")

(display "Booting partial document facilities\n")
(lazy-define (part part-shared) buffer-initialize buffer-notify)
;(lazy-menu (part part-menu) document-master-menu)
(lazy-tmfs-handler (part part-tmfs) part)
;(display* "time: " (- (texmacs-time) boot-start) "\n")
;(display* "memory: " (texmacs-memory) " bytes\n")

(display "Booting fonts\n")
(use-modules (fonts fonts-ec) (fonts fonts-adobe) (fonts fonts-x)
             (fonts fonts-math) (fonts fonts-foreign) (fonts fonts-misc)
             (fonts fonts-composite) (fonts fonts-truetype))


;; additional markup functions
(use-modules (utils misc markup-funcs))


(set-new-fonts #t)

  
(define (standard-paper-size s) s)
(define (gui-version) "none")
(define (image->psdoc a) "")


(display "****** End booting init-vau.scm\n")


(display "------------------------------------------------------\n")
(delayed (:idle 10000) (autosave-delayed))
(texmacs-banner)
(display "Initialization done\n")

(let ()
  (display "------------------------------------------------------\n")
  (display "Benchmark 2\n")
  (define start (texmacs-time))
  (tm-define (tm-fib n) (if (< n 2) n (+ (tm-fib (- n 1)) (tm-fib (- n 2)))))
  (display (tm-fib 30))
  (newline)
  (display "Time: ") (display (- (texmacs-time) start)) (newline)
  (display "------------------------------------------------------\n")
)


(tm-define (benchmark-menu-expand)
  (display "------------------------------------------------------\n")
  (display "Benchmark menu-expand\n")
  (let ((start (texmacs-time)))
  (display (menu-expand '(horizontal (link texmacs-main-icons))))
  (newline)
  (display "Time: ") (display (- (texmacs-time) start)) (newline))
  (display "------------------------------------------------------\n")
)

;; (delayed (:idle 1000) (benchmark-menu-expand))

;; you can run
;;   texmacs.bin -x "(benchmark-manual)"
;; to run this test

(tm-define (benchmark-manual)
(exec-delayed (lambda ()
(let ((root (url-resolve (url-unix "$TEXMACS_DOC_PATH" "main/man-manual.en.tm") "r"))
      (start-time (texmacs-time))
      (update (lambda (cont)
                (generate-all-aux)
                (update-current-buffer)
                (exec-delayed cont))))
  (tmdoc-expand-help root "book")
  (exec-delayed
    (lambda ()
      (update
        (lambda ()
          (update
            (lambda ()
              (update
                (lambda ()
                  (buffer-pretend-saved (current-buffer))
                  (display "Timing:") (display (- (texmacs-time) start-time)) (newline)
                  ;(quit-TeXmacs)
                  ))))))))))))

