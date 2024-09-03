# Diário de Bordo
## Legendas

|Símbolo|Significado|
|:--|:---|
|:checkered_flag:|Conquista, Milestone
|:exclamation:| Problema|
:sparkles:| Momento Eureka
:coffee:| Pausa

## :blue_square: 28/08:
- Clonei o repositório do xv6 e regredi o repositório de volta para a versão do commit rev11 pedida no EP.
- Instalei uma máquina virtual com Lubuntu para testar a execução do SO. Os pacotes necessários também foram instalados.

## :blue_square: 30/08:
- Criei um caminho de compartilhamento de arquivos entre a máquina e o host Windows.
- :exclamation: O xv6 consegue ser compilado e executado perfeitamente na máquina virtual, mas não quando armazenado na pasta de compartilhamento. Precisamos de outra alternativa...
- :exclamation: A depuração remota do GDB funciona com o QEMU, mas a depuração do VSCode não funciona com o QEMU.
- Ou seja, precisamos achar outra maneira de compilar e de depurar o xv6 dentro do Window.

## :blue_square: 31/08:
- Instalei o WSL2 para executar um cliente leve com Ubuntu e depurar o xv6 diretamente no Windows.
- :exclamation: Por alguma razão, a compilação no WSL2 gera um executável não bootável, apesar de compilar tudo. Mas a compilação na máquina virtual com Lubuntu funciona?? Existe alguma diferença entre as duas máquinas...

## :blue_square: 01/08:
- :sparkles: A diferença aparentemente era a versão do Binutils! Não só a versão do GCC importa como a versão do Binutils também. O WSL vem por padrão com o Ubuntu 22.04 LTS, e o Binutils 2.38. A máquina do Lubuntu executa a versão 24.0.4 LTS, com o Binutils 2.42. Após atualizar o WSL2 para a versão mais nova do Ubuntu e o Binutils mais recente, a compilação e execução funcionam no WSL2!
- Agora precisamos encontrar como depurar o xv6. O gdb do WSL2 já vem funcional e consegue interagir apropriadamente com o stub do GDB no QEMU, mas não quero depurar na mão, preciso depurar no VSCode.
- Mesmo configurando o VSCode para conectar com o stub remoto em :26000, o QEMU crasha assim que a conexão é feita, suspeito que seja uma arquitetura binária incompatível entre o GDB instalado no Windows e o stub do xv6.
- Encontrei uma extensão do VS que permite executá-lo dentro do WSL2, e executar o GDB instalado dentro do WSL2. Isso elimina a incompatibilidade arquitetural. Dito isso, o QEMU ainda crasha quando me conecto através do depurador do VS.
- :coffee: Pausa para o café.

- :sparkles: O QEMU crashava pois, ao iniciar o gdb na pasta do xv6, o xv6 executa um script .gdbinit. Esse script, entre outras coisas, **JÁ** especifica o comando 'target remote :26000'. Ao tentar especificar o comando denovo, o gdb termina o processo anterior. Ou seja, o VSCode iniciava o gdb na pasta, o .gdbinit iniciava uma conexão com o stub em :26000, e depois o VS tentava se conectar mais uma vez a :26000, terminando o QEMU. A solução foi apenas reconfigurar launch.json de forma que o VS apenas inicie o gdb na pasta do xv6, sem nenhuma configuração adicional.
- A depuração finalmente funciona, mas resta ainda testar a funcionalidade dos breakpoints, além de configurar o ambiente de desenvolvimento, para não ter que compilar ou rodar nada diretamente no terminal.
- :checkered_flag: Os breakpoints do kernel e dos programas de usuário funcionam apropriadamente. Estamos prontos para começar.

## :blue_square: 02/08:
- Precisamos agora descobrir quais arquivos estão do lado do kernel e quais estão do lado do usuário, além de descobrir quais a participação de cada arquivo relevante na chamada do sistema.

Antes de tudo, o cabeçalho _**syscall.h**_ é usado tanto do lado do Kernel como do lado do usuário, e possui definições de constantes para cada tipo de syscall utilizada no sistema. Essa tabela de constantes é depois consultada para obter o número da chamada desejada pelo seu nome.

Do lado do Kernel, temos os seguintes arquivos:
Arquivo|Objetivo
:---|:--
syscall.c|Implementa a tabela de chamadas de sistema que mapeia os números para as funções. Os handlers que recebem a chamada diretamente sempre são prefixados com sys_, retornam um tipo int e não recebem argumentos.
sysproc.c|Implementação dos handlers das syscalls relacionadas a manuseamento de processos e memória de processos.
sysfile.c|Implementação das syscalls relacionadas ao sistema de arquivos.

Do lado do usuário, são os seguintes arquivos principais:

Arquivo|Funcionalidade
:---|:---
user.h|Protótipos das funções principais do ambiente de usuário. Inclui tanto os protótipos das syscalls como algumas chamadas da pseudo-biblioteca C.
usys.S|Implementação em Assembly das funções syscalls declaradas em user.h. Note que a implementação não processa, organiza ou verifica nenhum parâmetro. Essas responsabilidades ficam do lado do kernel.
ulib.c|Implementação de algumas funcões da biblioteca C. Algumas operam apenas em memória e outras realizam chamadas de sistema.

:sparkles: Agora, é possível entender todo o roteiro do processo de chamada de uma syscall e como o controle da CPU passa de um código para outro. Os passos principais (e que teremos que alterar em algum momento) são os seguintes:
1. O programa de usuário chama alguma função declarada em _user.h_, ex. ```getpid()```.
2. A implementação de ```getpid()``` está em _usys.S_, a função apenas coloca o número da syscall pedida (números declarados em _syscall.h_) no registrador EAX e realiza o interrupt 64.
3. Através do gate declarado na tabela de vetores e inicializado pelo Kernel no boot, o controle é transferido para o handler de traps no Kernel.
4. O handler de traps interpreta o número da interrupção e passa o controle para a função ```syscall()``` implementada em _syscall.c_. Essa função procura na tabela de chamadas, valida o número da syscall executada, e finalmente, passa o controle para o handler apropriado.
5. Para a chamada de sistema nso exemplo, o handler chamado é o ```sys_getpid()```, definido em _sysproc.c_. A função tem uma linha só, que obtém a estrutura do processo atual e obtém o seu ID numérico.
6. O controle retorna do kernel através do gate de traps e volta para o ambiente de usuário com o id numérico do processo.

A chamada de sistema getpid() é simples pois não recebe qualquer parâmetro nem realiza operações muito complexas. Para outras syscalls, grande parte do código implementado consiste em processar os parâmetros que estão na pilha.

- :coffee: Momento do café

---

Estamos prontos para implementar a chamada do sistema.
### Lado do Kernel
Vamos começar atribuindo um novo número para a chamada na tabela global de chamadas declarada em _syscall.h_, definirei a nova syscall com essa linha:
```c
#define SYS_getreadcount 22
```
Vamos agora criar um stub para o handler da nova syscall e adicioná-lo na tabela de _syscall.c_.
Nesse arquivo, criei o seguinte protótipo: 
```c
extern int sys_getreadcount();
```
Como essa nova chamada lida com informações do processo, irei implementá-la em _sysproc.c_. Temporariamente, para testar a nova syscall, vou fazê-la retornar um valor simbólico com a seguinte implementação:
```c
int sys_getreadcount() {
  return 1234567;
}
```
O lado do Kernel está pronto, precisamos agora adicionar as chamadas do lado do usuário.
### Lado do Usuário
Primeiro, adicionarei mais uma linha em _usys.S_ que define uma função que ativa o interrupt da syscall.
```
SYSCALL(getreadcount)
```
Agora, em _user.h_, vamos definir o protótipo da nova chamada de sistema. Adicionei o seguinte protótipo:
```c
int getreadcount(void);
```
Para testar essas implementações, fiz uma modificação no programa **cat.c**, para testar a impressão do número teste 1234567.
```c
int n = getreadcount();
printf(1, "this is a number: %d\n", n);
```

:checkered_flag: O teste funcionou! A chamada do sistema funciona sem crashes e imprime o número correto.

## :blue_square: 03/09:
Precisamos agora criar um contador global para o valor. Esse contador deve ser uma variável global no sistema e possivelmente uma variável volátil a depender se o Kernel é sincronizado entre mais de uma CPU ou não.
- Tudo ficaria mais fácil se a chamada de sistema que incrementa o contador e a chamada que lê o contador ficassem no mesmo arquivo.
- Dito isso, é uma boa ideia mover a nossa chamada ```sys_getreadcount()``` de _sysproc.h_ para _sysread.c_.
- Movi a chamda de sistema para o outro arquivo e ela continua funcionando perfeitamente.
