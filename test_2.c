#include "types.h"
#include "stat.h"
#include "user.h"

// teste da nova chamada de sistema em uma situação em que ha' concorrencia
//      (pode ser preciso usar uma lock para garantir que nao havera erros)
int main(int argc, char *argv[]) {
    int x1 = getreadcount();

    int rc = fork();

    int total = 0;
    int i;
    for (i = 0; i < 100000; i++) {
    	char buf[100];
    	(void) read(4, buf, 1);
    }

    if (rc > 0) {
    	(void) wait();
    	int x2 = getreadcount();
    	total += (x2 - x1);
    	printf(1, "XV6_TEST_OUTPUT %d\n", total);
        // saida esperarda:
        // XV6_TEST_OUTPUT 200000
    }
    exit();
}
