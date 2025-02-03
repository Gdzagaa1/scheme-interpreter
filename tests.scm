;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; TESTS ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;TEST1

(if (equal? 3 ((lambda (x y) (+ x y)) 1 2)) "TEST1: LAMBDA - SUCCESS" "TEST1: LAMBDA - FAIL")

;;;;;;;TEST2

(define pi 3.14)
(if (equal? pi 3.14) "TEST2: DEFINE - SUCCESS" "TEST2: DEFINE - FAIL")

;;;;;;;TEST3

(define (sum x y) (+ x y))
(if (equal? (sum 5 6) 11) "TEST3: DEFINE - SUCCESS" "TEST3: DEFINE - FAIL")

;;;;;;;TEST4

(if (equal? (+ 1 3 4 5) 13) "TEST4: PLUS - SUCCESS" "TEST4: PLUS - FAIL")

;;;;;;;TEST5

(if (equal? (* 1 3 4 5) 60) "TEST5: MULTIPLICATION - SUCCESS" "TEST5: MULTIPLICATION - FAIL")

;;;;;;;TEST6

(if (equal? (/ 20 2 6) 5/3) "TEST6: DIVISION - SUCCESS" "TEST6: DIVISION FAIL")

;;;;;;;TEST7

(if (equal? (- 1 3 4 5) -11) "TEST7: MINUS - SUCCESS" "TEST7: MINUS - FAIL")

;;;;;;;TEST8

(if (equal? (and 1 1) 1) "TEST8: AND - SUCCESS" "TEST8: AND - FAIL")

;;;;;;;TEST9 

(if (equal? (or 1 0) 1) "TEST9: OR - SUCCESS" "TEST9: OR - FAIL")

;;;;;;;TEST10

(if (equal? (car(cdr '(1 2 3))) 2) "TEST10: CAR_CDR - SUCCESS" "TEST10: CAR_CDR - FAIL")

;;;;;;;TEST11

(if (equal? (cons 1 '(2 3)) '(1 2 3)) "TEST11: CONS - SUCCESS" "TEST11: CONS - FAIL")

;;;;;;;TEST12

(if (equal? '(1 4 9) (map (lambda (x) (* x x)) '(1 2 3))) "TEST12: MAP_LAMBDA - SUCCESS" "TEST12: MAP_LAMBDA - FAIL")

;;;;;;;TEST13

(if (equal? '(1 2 3 4) (append '(1 2) '(3 4))) "TEST13: APPEND - SUCCESS" "TEST13: APPEND - FAIL")

;;;;;;;TEST14

(if (equal? 6 (apply + '(1 2 3))) "TEST14: APPLY - SUCCESS" "TEST14: APPLY - FAIL")

;;;;;;;TEST15

(if (equal? 30 (eval (apply (lambda (x y) (* x y)) '(5 6)))) "TEST15: EVAL_APPLY_LAMBDA - SUCCESS" "TEST15: EVAL_APPLY_LAMBDA - FAIL")

;;;;;;;TEST16

(if (equal? 1 (null? '())) "TEST16: NULL? - SUCCESS" "TEST16: NULL? - FAIL")

;;;;;;;TEST17

(if (equal? 1 (length? '(7))) "TEST17: LENGTH - SUCCESS" "TEST17: LENGTH - FAIL")

;;;;;;;TEST18

(define (fact n)  (if (= n 0) 1 (* n (fact (- n 1)))))

(if (equal? 120 (fact 5)) "TEST19: RECURSION - SUCCESS" "TEST19: RECURSION - FAIL")
