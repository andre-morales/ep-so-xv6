# Diário de Bordo
### Legendas

|Símbolo|Significado|
|:--|:---|
|:checkered_flag:|Conquista, Milestone
|:exclamation:| Problema|
:sparkles:| Momento Eureka
:coffee:| Pausa

**28/08:**
- Clonei o repositório do xv6 e regredi o repositório de volta para a versão do commit rev11 pedida no EP.
- Instalei uma máquina virtual com Lubuntu para testar a execução do SO. Os pacotes necessários também foram instalados.

**30/08:**
- Criei um caminho de compartilhamento de arquivos entre a máquina e o host Windows.
- :exclamation: O xv6 consegue ser compilado e executado perfeitamente na máquina virtual, mas não quando armazenado na pasta de compartilhamento. Precisamos de outra alternativa...
- :exclamation: A depuração remota do GDB funciona com o QEMU, mas a depuração do VSCode não funciona com o QEMU.
- Ou seja, precisamos achar outra maneira de compilar e de depurar o xv6 dentro do Window.

**31/08:**
- Instalei o WSL2 para executar um cliente leve com Ubuntu e depurar o xv6 diretamente no Windows.
- :exclamation: Por alguma razão, a compilação no WSL2 gera um executável não bootável, apesar de compilar tudo. Mas a compilação na máquina virtual com Lubuntu funciona?? Existe alguma diferença entre as duas máquinas...

**01/08:**
- :sparkles: A diferença aparentemente era a versão do Binutils! Não só a versão do GCC importa como a versão do Binutils também. O WSL vem por padrão com o Ubuntu 22.04 LTS, e o Binutils 2.38. A máquina do Lubuntu executa a versão 24.0.4 LTS, com o Binutils 2.42. Após atualizar o WSL2 para a versão mais nova do Ubuntu e o Binutils mais recente, a compilação e execução funcionam no WSL2!
- Agora precisamos encontrar como depurar o xv6. O gdb do WSL2 já vem funcional e consegue interagir apropriadamente com o stub do GDB no QEMU, mas não quero depurar na mão, preciso depurar no VSCode.
- Mesmo configurando o VSCode para conectar com o stub remoto em :26000, o QEMU crasha assim que a conexão é feita, tenho duas suspeitas, suspeito que seja uma arquitetura binária incompatível entre o GDB instalado no Windows e o stub do xv6.
- Encontrei uma extensão do VS que permite executá-lo dentro do WSL2, e executar o GDB instalado dentro do WSL2. Isso elimina a incompatibilidade arquitetural. Dito isso, o QEMU ainda crasha quando me conecto através do depurador do VS.
- :coffee: Pausa para o café.

- :sparkles: O QEMU crashava pois, ao iniciar o gdb na pasta do xv6, o xv6 executa um script .gdbinit. Esse script, entre outras coisas, **JÁ** especifica o comando 'target remote :26000'. Ao tentar especificar o comando denovo, o gdb termina o processo anterior. Ou seja, o VSCode iniciava o gdb na pasta, o .gdbinit iniciava uma conexão com o stub em :26000, e depois o VS tentava se conectar mais uma vez a :26000, terminando o QEMU. A solução foi apenas reconfigurar launch.json de forma que o VS apenas inicie o gdb na pasta do xv6, sem nenhuma configuração adicional.
- A depuração finalmente funciona, mas resta ainda testar a funcionalidade dos breakpoints, além de configurar o ambiente de desenvolvimento, para não ter que compilar ou rodar nada diretamente no terminal.
