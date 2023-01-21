
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; MODULE      : init-vau.scm
;; DESCRIPTION : This is the Vau initialization file
;; COPYRIGHT   : (C) 2022 Massimiliano Gubinelli
;;
;; This software falls under the GNU general public license version 3 or later.
;; It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
;; in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(cond ((os-mingw?)
       (debug-set! stack 0))
      ((os-macos?)
       (debug-set! stack 2000000))
      (else
       (debug-set! stack 1000000)))

(define boot-start (texmacs-time))
(define remote-client-list (list))

(define developer-mode?
  (equal? (cpp-get-preference "developer tool" "off") "on"))

(if developer-mode?
    (debug-enable 'backtrace 'debug))

(define (%new-read-hook sym) (noop)) ; for autocompletion

(define-public macro-keywords '(define-macro define-public-macro
                                tm-define-macro))
(define-public def-keywords
  `(define-public provide-public
    tm-define tm-menu menu-bind tm-widget ,@macro-keywords))

(define old-read read)
(define (new-read port)
  "A redefined reader which stores line number and file name in symbols."
  ;; FIXME: handle overloaded definitions
  (let ((form (old-read port)))
    (if (and (pair? form) (member (car form) def-keywords))
        (let* ((l (source-property form 'line))
               (c (source-property form 'column))
               (f (source-property form 'filename))
               (sym  (if (pair? (cadr form)) (caadr form) (cadr form))))
          (if (symbol? sym) ; Just in case
              (let ((old (or (symbol-property sym 'defs) '()))
                    (new `(,f ,l ,c)))
                (%new-read-hook sym)
                (if (and (member (car form) macro-keywords)
                         (not (member sym def-keywords)))
                    (set! def-keywords (cons sym def-keywords)))
                (if (not (member new old))
                    (set-symbol-property! sym 'defs (cons new old)))))))
    form))

(define old-primitive-load primitive-load)
(define (new-primitive-load filename)
  (if (member (scheme-dialect) (list "guile-a" "guile-b"))
      (old-primitive-load filename)
      ;; We explicitly circumvent guile's decision to set the current-reader
      ;; to #f inside ice-9/boot-9.scm, try-module-autoload
      (with-fluids ((current-reader read))
                   (old-primitive-load filename))))

(if developer-mode?
    (begin
      (module-export! (current-module)
                      '(%new-read-hook old-read new-read def-keywords))
      (set! read new-read)
      (module-export! (current-module)
                      '(old-primitive-load new-primitive-load))
      (set! primitive-load new-primitive-load)))

;; TODO: scheme file caching using (set! primitive-load ...) and
;; (set! %search-load-path)

;;(debug-enable 'backtrace 'debug)
;; (define load-indent 0)
;; (define old-primitive-load primitive-load)
;; (define (new-primitive-load . x)
;;   (for-each display (make-list load-indent "  "))
;;   (display "Load ") (apply display x) (display "\n")
;;   (set! load-indent (+ load-indent 1))
;;   (apply old-primitive-load x)
;;   (set! load-indent (- load-indent 1))
;;   (for-each display (make-list load-indent "  "))
;;   (display "Done\n"))
;; (set! primitive-load new-primitive-load)

(display "Booting Vau kernel functionality\n")
(load (url-concretize "$TEXMACS_PATH/progs/kernel/boot/boot.scm"))
(inherit-modules (kernel boot compat) (kernel boot abbrevs)
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
;                 (kernel gui kbd-define)
;                 (kernel gui speech-define)
;                 (kernel gui kbd-handlers)
;                 (kernel gui menu-test)
;                 (kernel old-gui old-gui-widget)
;                 (kernel old-gui old-gui-factory)
;                 (kernel old-gui old-gui-form)
;                 (kernel old-gui old-gui-test))
;(display* "time: " (- (texmacs-time) boot-start) "\n")
;(display* "memory: " (texmacs-memory) " bytes\n")

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



(set-new-fonts #t)

(define (notify-debug-message channel)
  (noop))
  
(define (standard-paper-size s) s)
(define (gui-version) "none")
(define (image->psdoc a) "")


(display "****** End booting init-vau.scm\n")
