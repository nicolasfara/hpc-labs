# Report on Excercise 2

### omp-matmul

Si vanno ad effettuare vari tentativi per quanto riguarda l'efficienza:

* Usando la clausola `collapse(2)`
* Senza la clausola `collapse(2)`
* Utilizzare schedule **dynamic**
  - Testare vari blocks
* Utilizzare schedule **static**

| schedule | block 2  | block 4  | block 8  | block 16 | block 32 |
| -------- | -------- | -------- | -------- | -------- | -------- |
| dynamic  | 3.472926 | 3.662534 | 3.942630 | 3.900198 | 3.79709  |
| static   | 3.656575 | 3.860959 | 3.862119 | 3.873866 | 4.04918  |
