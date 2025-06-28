# ppos
repositório do projeto PingPongOS de Sistemas Operacionais da UTFPR

**Comandos para teste**
PROJETO A--------------------//////////
Teste do scheduler: gcc -Wall ppos_solucao.c ppos-core-aux.c pingpong-scheduler.c queue.o disk-driver.o ppos-all.o ppos-disk-manager.o -o scheduler_test -lrt
Execução: ./scheduler_test

Teste da preempção: gcc -Wall ppos_solucao.c ppos-core-aux.c pingpong-preempcao.c queue.o disk-driver.o ppos-all.o ppos-disk-manager.o -o preemption_test -lrt
Execução: ./preemption_test

Teste da contabilização de métricas: gcc -Wall ppos_solucao.c ppos-core-aux.c pingpong-contab-prio.c queue.o disk-driver.o ppos-all.o ppos-disk-manager.o -o contab_test -lrt
Execução: ./contab_test

PROJETO B-----------------------////////////////
Teste de disco1: gcc -Wall ppos_solucao.c ppos-core-aux.c ppos_disk.c pingpong-disco1.c queue.o disk-driver.o ppos-all.o -o disco_test1 -lrt 
