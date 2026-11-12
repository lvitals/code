# Code Editor: manual do usuário

*Read in English: [cedit.md](cedit.md).  A página de manual é
`man cedit` (em português: `man -M /usr/local/share/man/pt_BR cedit`,
ou `man -L pt_BR cedit` com o man-db; fontes em
[man/cedit.1](man/cedit.1) e [man/pt_BR/cedit.1](man/pt_BR/cedit.1)).*

O Code Editor é um editor de texto escrito em C99 portável.  Tem barra
de menus, suporte a mouse e os atalhos de teclado usuais.  São dois
programas compilados das mesmas fontes:

- **`cedit`** roda num terminal;
- **`xcedit`** roda numa janela própria (GLFW e OpenGL), com o texto
  desenhado por uma fonte embutida no programa (FantasqueSansMono) ou
  por outra escolhida nas configurações.

Os dois têm os mesmos menus, teclas, diálogos e configurações.  Tudo o
que está descrito aqui vale para ambos, salvo indicação em contrário.

## Sumário

1. [Compilar e instalar](#1-compilar-e-instalar)
2. [Iniciar o editor](#2-iniciar-o-editor)
3. [A tela](#3-a-tela)
4. [Mover e selecionar](#4-mover-e-selecionar)
5. [Editar](#5-editar)
6. [Localizar e Substituir](#6-localizar-e-substituir)
7. [Navegar](#7-navegar)
8. [Arquivos: Abrir, Salvar Como e os diálogos](#8-arquivos-abrir-salvar-como-e-os-diálogos)
9. [O projeto: Open Project Folder, Open File in Project, Search in Project](#9-o-projeto)
10. [Codificações e fins de linha](#10-codificações-e-fins-de-linha)
11. [O menu View](#11-o-menu-view)
12. [Realce de sintaxe e linguagens](#12-realce-de-sintaxe-e-linguagens)
13. [Arquivos grandes](#13-arquivos-grandes)
14. [A janela (xcedit)](#14-a-janela-xcedit)
15. [Configurações](#15-configurações)
16. [Referência de teclas](#16-referência-de-teclas)
17. [Arquivos e ambiente](#17-arquivos-e-ambiente)

Os nomes de menus, itens e botões estão em inglês, como aparecem no
editor.

---

## 1. Compilar e instalar

Requisitos:

- `cedit`: um compilador C99 e um sistema POSIX (termios, dirent, poll,
  threads POSIX).
- `xcedit`: também GLFW 3 e OpenGL.  O stb_truetype, que desenha os
  caracteres, faz parte das fontes.

Os sistemas: Linux, macOS, FreeBSD, OpenBSD e NetBSD, das mesmas
fontes (POSIX e C99).  O Makefile serve para qualquer make POSIX, então
o `make` de cada sistema o compila; no macOS o OpenGL é o framework do
sistema.

```
make                 # build/cedit e build/xcedit
make cedit           # só o editor de terminal (não precisa de GLFW)
make xcedit          # o editor em janela
make CC=clang        # outro compilador C99 (CC=alefc também funciona)
make clean
```

Instalação:

```
make install                    # em /usr/local (como root)
make install PREFIX=~/.local    # para um usuário
make install DESTDIR=/tmp/pkg   # em um diretório de pacote
make uninstall
```

São instalados o `cedit` e o `xcedit`, as páginas de manual (`cedit(1)`,
com `xcedit(1)` como link para ela, em inglês e em português), a entrada
do xcedit no menu do sistema (`share/applications/xcedit.desktop`, com o
nome "Code Editor") e o ícone
(`share/icons/hicolor/scalable/apps/xcedit.svg`).

## 2. Iniciar o editor

```
cedit  [OPÇÕES] [ARQUIVO|PASTA]...
xcedit [OPÇÕES] [ARQUIVO|PASTA]...
cedit  --lex ARQUIVO [LINGUAGEM]
```

Cada ARQUIVO é aberto num documento próprio.  Um ARQUIVO que não existe
abre como documento novo e vazio com esse nome, e é criado ao salvar.
Sem ARQUIVO, abre um documento vazio sem nome.

Uma PASTA não é aberta como arquivo: ela passa a ser a **pasta do
projeto** (veja a [seção 9](#9-o-projeto)).  `cedit .` abre o editor com a
pasta atual como projeto, sem mostrar diálogo.

Opções:

| Opção | Significado |
|---|---|
| `-f`, `--foreground` | xcedit: mantém o terminal preso até a janela fechar.  Use quando o xcedit for o `EDITOR` (por exemplo no `git commit`, que espera o editor). |
| `-g`, `--goto ARQUIVO:LINHA[:COLUNA]` | Abre o ARQUIVO com o cursor na LINHA (e COLUNA).  Contam a partir de 1. |
| `-n`, `--no-line-numbers` | Abre os arquivos sem números de linha.  As linhas nem são contadas, o que ajuda com arquivos muito grandes. |
| `-h`, `--help` | Mostra o uso e sai. |
| `-v`, `--version` | Mostra a versão e sai. |
| `--lex ARQUIVO [LINGUAGEM]` | Imprime cada linha do ARQUIVO e, embaixo, a classe de realce de cada caractere, e sai.  Para quem escreve uma linguagem; veja `docs/LANGUAGES.md`. |
| `--` | Os argumentos seguintes são nomes de arquivo, mesmo que comecem com `-`. |

As letras do `--lex`: `.` texto, `K` palavra-chave, `T` tipo ou função,
`S` string, `C` comentário, `N` número ou constante, `P` pré-processador,
`D` linha removida (diffs).

**O xcedit e o terminal.**  Iniciado de um terminal, o xcedit devolve o
prompt assim que a janela abre.  A janela continua independente do
terminal, e fechar o terminal não a fecha.  Se a janela não puder ser
aberta, o motivo é mostrado no terminal.  Com `-f` o terminal espera a
janela fechar.

Exemplos:

```
cedit notas.txt
cedit -g src/main.c:120:5
xcedit ~/projetos/site          # essa pasta é o projeto
EDITOR="xcedit -f" git commit
cedit --lex teste.lua lua
```

## 3. A tela

De cima para baixo:

- **A barra de menus**: File, Edit, View, Help.  Alt e a letra
  sublinhada abre um menu, assim como F10.  No menu, as setas movem,
  Enter ou a letra do item escolhe e Esc fecha.  Cada item mostra seu
  atalho.  Itens que não se aplicam (Save sem documento, por exemplo)
  aparecem em cinza.
- **A área de texto**: os números de linha na margem, o texto, a barra
  de rolagem vertical à direita e, sem Word Wrap, uma barra horizontal
  embaixo do texto.
- **A barra de busca**, quando aberta (seção 6).
- **A barra de status**.

### A barra de status

Da esquerda para a direita, conforme a largura permite:

| Item | Significado | Um clique |
|---|---|---|
| Linguagem | A linguagem do documento, para o realce | |
| LF / CRLF | O fim de linha | Alterna entre os dois |
| Codificação | UTF-8, UTF-16LE, windows-1252, … | Abre Reopen with encoding e Convert to encoding (seção 10) |
| Indentação | Tabs, ou espaços e quantos, como detectado no arquivo | Muda a indentação |
| Linha:Coluna | A posição do cursor | |
| OVR | Modo de sobrescrever ligado (tecla Insert) | Desliga |
| `*` | O documento tem alterações não salvas | |
| Projeto | O nome da pasta do projeto (à direita) | Abre o Open Project Folder |
| Nome do arquivo | O nome do documento | Go to File (troca de documento) |

Numa tela estreita, os itens que não cabem saem por ordem de
importância e rótulos mais curtos são usados.  View > Focus Statusbar
leva o foco do teclado para a barra, para usar os botões sem mouse.

### Diálogos

Todos os diálogos funcionam igual.  Tab e Shift+Tab passam entre os
itens (campos, listas, caixas de seleção, botões).  Enter faz a ação
padrão, Esc fecha, e o mouse funciona em todos os itens.

## 4. Mover e selecionar

| Tecla | Ação |
|---|---|
| Setas | Um caractere ou uma linha |
| Ctrl+Esquerda / Ctrl+Direita | Uma palavra |
| Home | Primeiro caractere não branco da linha; de novo, o início da linha |
| End | Fim da linha |
| PgUp / PgDn | Uma tela acima ou abaixo |
| Ctrl+Home / Ctrl+End | Início ou fim do documento |
| Ctrl+Cima / Ctrl+Baixo | Rola a vista uma linha sem mover o cursor |
| Shift + qualquer uma acima | Estende a seleção |
| Ctrl+A | Seleciona tudo |
| Ctrl+L | Seleciona a linha atual (de novo: acrescenta a próxima) |
| Shift+Alt+Direita | Expand Selection: a palavra, o conteúdo das aspas, as aspas, o conteúdo dos parênteses, os parênteses, a linha, tudo |
| Shift+Alt+Esquerda | Shrink Selection: volta um passo |

O mouse:

- um clique posiciona o cursor; arrastar seleciona, e rola quando passa
  da borda da área de texto;
- clique duplo seleciona uma palavra; clique triplo, a linha;
- Shift+clique estende a seleção;
- a roda rola; Shift+roda, ou a roda lateral, rola as colunas;
- o botão do meio cola;
- o botão direito abre um menu de contexto (Cut, Copy, Paste, Select
  All, Find);
- nas barras de rolagem, as setas rolam, um clique na trilha salta e o
  marcador pode ser arrastado.

## 5. Editar

### Digitação

- O texto digitado é inserido, ou, no modo de sobrescrever (tecla
  Insert, indicado por OVR), substitui o que está lá.
- Enter mantém a indentação da linha (Auto Indent, ligado por padrão).
- Backspace na indentação remove um nível inteiro quando "Backspace
  Removes a Whole Indent" está ligado; senão, um caractere.
- Tab insere um tab ou espaços, conforme a indentação do documento.  Com
  uma seleção de várias linhas, Tab indenta todas e Shift+Tab remove um
  nível.
- Ctrl+Backspace e Ctrl+Delete apagam uma palavra.
- Ctrl+Shift+H apaga até o início da linha; Ctrl+Shift+G, até o fim.
- Alt+Cima e Alt+Baixo movem a linha atual (ou as selecionadas) para
  cima ou para baixo.

### Área de transferência

| Tecla | Ação |
|---|---|
| Ctrl+X, Shift+Delete | Recortar (sem seleção: a linha inteira) |
| Ctrl+C, Ctrl+Insert | Copiar (sem seleção: a linha inteira) |
| Ctrl+V, Shift+Insert | Colar |

No terminal, o editor copia com OSC 52, de modo que o texto também chega
à área de transferência do sistema quando o terminal suporta, e aceita
o bracketed paste.  Na janela, usa diretamente a área de transferência
do sistema.

### Desfazer e refazer

Ctrl+Z desfaz e Ctrl+Y (ou Ctrl+Shift+Z) refaz.  Caracteres digitados em
sequência são desfeitos juntos.

### Múltiplos cursores

- Alt+clique acrescenta um cursor, ou remove um que já está lá.
- Ctrl+D seleciona a palavra no cursor; de novo, acrescenta um cursor
  na próxima ocorrência da seleção (Edit > Add Next Occurrence).
- Shift+Alt+Cima e Shift+Alt+Baixo acrescentam um cursor na linha acima
  ou abaixo.

Digitação, Backspace, Delete, Enter, Tab, as setas, Home, End, recortar,
copiar e colar agem em todos os cursores.  Copiar com N cursores e colar
com N cursores põe uma linha em cada.  Esc volta a um só cursor.

### Complete Word (Ctrl+Space)

Completa a palavra antes do cursor com as palavras do documento, as
mais próximas primeiro.  Se só há uma candidata, ela entra na hora.
Senão, abre uma lista no cursor: digitar filtra, Cima e Baixo escolhem,
Enter ou Tab inserem, Esc fecha.

### Filter Through Command (Alt+|)

Pede um comando do shell e o executa com `sh -c`.  A seleção vai para a
entrada padrão do comando e é substituída pelo que ele escreve.  Sem
seleção, a saída é inserida no cursor.  Exemplos: `sort`, `fmt -w 72`,
`jq .`, `date`.  Se o comando falhar, nada muda e a mensagem de erro é
mostrada.  Um único desfazer volta a mudança inteira.

### A marca

Alt+M marca a posição do cursor; Alt+J volta para ela.

## 6. Localizar e Substituir

Ctrl+F abre a barra Find e Ctrl+R abre Find e Replace.  Com um texto
selecionado (dentro de uma linha), a seleção vira o texto a procurar.

- **Find**: a busca acompanha a digitação.  Enter, ou F3, vai para a
  próxima ocorrência.  O campo muda de cor quando não há resultado.
- **Replace**: Enter no campo Replace substitui a ocorrência atual e vai
  para a próxima.  Replace All (botão, ou Ctrl+Alt+Enter no campo)
  substitui todas; um desfazer volta tudo.
- **Match Case**, **Whole Word** e **Use Regex** são caixas de seleção,
  lembradas nas configurações.
- **Close**, ou Esc, fecha a barra.

Sem Match Case, as letras casam em maiúsculas e minúsculas.

### Expressões regulares

O editor tem seu próprio motor de expressões regulares
(`src/core/regex.c`).

| Sintaxe | Casa com |
|---|---|
| `.` | qualquer caractere menos o fim de linha |
| `[abc]`, `[a-z]`, `[^…]` | um caractere do conjunto (ou fora dele) |
| `\d` `\w` `\s` | dígito, caractere de palavra, espaço; `\D` `\W` `\S` o contrário |
| `\b` `\B` | limite de palavra, não limite de palavra |
| `^` `$` | início e fim de linha (`$` também casa antes de um CR LF) |
| `*` `+` `?` | 0 ou mais, 1 ou mais, 0 ou 1 |
| `{m}` `{m,}` `{m,n}` | exatamente m, ao menos m, de m a n |
| `*?` `+?` `??` `{…}?` | o mesmo, o mínimo possível (preguiçoso) |
| `( )` | grupo capturado (1 a 9) |
| `(?: )` | grupo não capturado |
| `a\|b` | um ou outro |
| `\n` `\t` `\r` `\.` … | o próprio caractere |

Na substituição, `$1` a `$9` (ou `\1` a `\9`) inserem os grupos e `$0` a
ocorrência inteira.

## 7. Navegar

| Tecla | Ação |
|---|---|
| Ctrl+G | Go to Line:Column (`120`, `120:5`) |
| Ctrl+P | Go to File: troca entre os documentos abertos |
| Ctrl+] | Vai ao parêntese correspondente, ou à outra ponta de um bloco (`if` … `end`, `#if` … `#endif`) |
| Alt+Esquerda | Go Back: volta para onde o cursor saltou |
| Alt+Direita | Go Forward |
| Alt+M / Alt+J | Marca / volta à marca |
| F3 | Próxima ocorrência |

Go Back e Go Forward lembram os saltos: Go to Line, uma busca, Ctrl+Home
e Ctrl+End, o parêntese correspondente, a marca, um clique distante e a
troca de documento.  As posições acompanham as edições.

## 8. Arquivos: Abrir, Salvar Como e os diálogos

| Tecla | Menu | Ação |
|---|---|---|
| Ctrl+N | File > New File | Documento novo, vazio |
| Ctrl+O | File > Open File… | Procura um arquivo para abrir |
| Ctrl+S | File > Save | Salva (pede um nome se o documento não tem) |
| Ctrl+Shift+S | File > Save As… | Salva com outro nome |
| Ctrl+W | File > Close File | Fecha o documento |
| Ctrl+Q | File > Exit | Sai do editor |

Close e Exit perguntam antes de descartar alterações não salvas (Save,
Don't Save, Cancel).

### Open File (Ctrl+O)

O diálogo mostra uma pasta (no início, a pasta do projeto), um campo
**Search** e uma lista: `..`, depois as pastas, depois os arquivos.

- Digitar em Search filtra a lista por busca aproximada: as letras
  digitadas precisam aparecer na ordem, não necessariamente juntas.
- Cima e Baixo (ou PgUp, PgDn, Ctrl+P, Ctrl+N) escolhem uma entrada,
  mesmo com o foco em Search.
- Enter numa pasta entra nela (`..` sobe); Enter num arquivo o abre.
- Backspace com Search vazio, ou Alt+Cima, sobe para a pasta de cima.
- Digitar `nome/` entra na pasta, `~/` vai para a pasta pessoal e `/`
  para a raiz; também se pode digitar um caminho.
- Se nada corresponde, o diálogo avisa, e Enter abre um arquivo novo com
  o nome digitado.
- Clique duplo abre; Tab passa entre Search, a lista e a caixa Hidden.
- Alt+H (Ctrl+H no xcedit), ou a caixa **Hidden**, mostra ou esconde os
  arquivos ocultos (nomes que começam com `.`).  A escolha fica nas
  configurações.

### Save As

O Save As tem um campo **File name** no lugar da busca, porque o nome
precisa ser digitado.  Abaixo dele aparecem os nomes da pasta que
começam com o que foi digitado.  Gravar por cima de um arquivo existente
pede confirmação.

### New Folder e Move to Trash

O Open File, o Save As e o Open Project Folder gerenciam as pastas do
mesmo jeito, com dois botões embaixo:

- **New Folder** (Alt+N): o campo pede o nome da pasta ("New folder:").
  Enter cria a pasta dentro da pasta mostrada e entra nela; Esc volta ao
  que estava no campo.  No Save As o nome de arquivo digitado antes é
  mantido, para salvar na pasta nova em seguida.
- **Trash (Del)**: move a pasta ou o arquivo escolhido na lista para a
  Lixeira,
  depois de uma confirmação (Yes, No).  O Del funciona na lista, ou no
  campo quando nada foi digitado (senão o Del edita o texto).

A Lixeira é a do próprio desktop (a especificação Trash do
freedesktop.org): `~/.local/share/Trash` (ou `$XDG_DATA_HOME/Trash`)
para o que está no mesmo disco da pasta pessoal; senão `.Trash-UID` no
topo do outro disco.  As pastas e os arquivos movidos aparecem na Lixeira do Arquivos do
GNOME, do Dolphin, do Thunar, do Caja e do Nemo, e podem ser
restauradas de lá.  O que é movido é renomeado, nunca copiado nem apagado.  O
próprio editor faz isso (POSIX: `rename`, `mkdir`, o `.trashinfo`
gravado), sem biblioteca de desktop nenhuma; num sistema sem desktop eles
ficam guardados nessa pasta do mesmo jeito.  No macOS a Lixeira
é a do Finder: `~/.Trash` (`.Trashes/UID` em outro disco).

Não são movidas, com o motivo informado: a pasta do projeto (ou uma
pasta que a contém), a pasta pessoal, uma pasta com um arquivo aberto
no editor e um arquivo aberto no editor.

### Explorer (Ctrl+E)

File > Explorer é um gerenciador de arquivos dentro do editor.  Como o
Open File, mostra a **Folder** (pasta) e uma linha **Search** (busca);
embaixo delas a pasta é uma árvore, e à direita há uma prévia do arquivo
sob o mouse (ou, pelo teclado, do escolhido), colorida como no Open File
in Project (Alt+P).  A prévia de uma pasta lista o que há nela.  A
rodinha sobre a prévia a rola.  A barra entre a árvore e a prévia
(marcada com ◀ ▶ no meio) pode ser arrastada com o mouse para a esquerda
ou para a direita, para ler caminhos longos; o Explorer sempre abre com
ela no lugar inicial.  Embaixo do botão [Project Folder] aparece o nome
do projeto, acinzentado, como na barra de status (`[code]`).  Ele sempre abre na pasta do projeto (a
do Open Project Folder); dali a pasta mostrada pode ser trocada (`..`,
Backspace, Open numa pasta, voltar e avançar) enquanto ele está aberto.

| Tecla, mouse | Ação |
|---|---|
| Up, Down, PgUp, PgDn, Home, End | Escolhe |
| Enter, clique duplo | Pasta: entra nela (vira a pasta mostrada); arquivo: abre no editor; `..`: a pasta de cima |
| Right / Left | Expande a pasta na árvore (de novo: vai à primeira entrada) / recolhe (de novo: vai à pasta de cima) |
| Clique numa pasta | Expande ou recolhe |
| Backspace, Alt+Up | A pasta de cima |
| Alt+Left / Alt+Right | Voltar / Avançar (as pastas mostradas antes) |
| Alt+O | Set as Project Folder: a pasta escolhida (num arquivo: a pasta mostrada) vira a pasta do projeto |
| Alt+Home, [Project Folder] | Volta à pasta do projeto (o botão fica à direita da linha Folder) |
| `nome/`, `~/`, `/` digitado | Entra nessa pasta (como no Open File) |
| Ctrl+X / Ctrl+C / Ctrl+V | Recortar / Copiar / Colar (na pasta escolhida, ou na pasta do arquivo escolhido) |
| F2 | Renomear |
| Del | Mover para a Lixeira (com confirmação) |
| Alt+F | New File: um arquivo vazio, com o nome inicial que o editor dá a um documento novo (`Untitled-1.txt`, o primeiro livre) |
| Alt+N | New Folder (pasta nova) |
| Alt+Enter | Properties (propriedades) |
| F5 | Lê as pastas de novo |
| Alt+C | Collapse All: recolhe todas as pastas da árvore |
| Alt+H | Mostra os arquivos ocultos |
| Botão direito, Shift+F10 | O menu daquela entrada (fora das entradas: da pasta mostrada) |
| Esc | Limpa a busca; sem nada digitado, fecha |
| Ctrl+E | Fecha |

O menu tem **Open** (um arquivo no editor; uma pasta passa a ser a
mostrada), **Set as Project Folder** (a pasta, ou fora das entradas a
pasta mostrada, passa a ser a do projeto, como no Open Project Folder),
**New File**, **New Folder**, **Cut**, **Copy**, **Paste**, **Copy To**
(copiar para), **Move To** (mover para), **Rename** (renomear), **Move
to Trash**, **Collapse All** (recolher tudo) e **Properties**; New File,
New Folder, Trash e Collapse All também são botões embaixo.  O que precisa
de um nome o pede na linha Search ("New file:", "New folder:", "New
name:", "Copy to:", "Move to:"): Enter
faz, Esc cancela.  Copy To e Move To começam com a pasta onde a entrada
está; uma pasta digitada ali recebe a entrada dentro dela, outro nome é
o novo nome (`~/` é a pasta pessoal; um caminho relativo parte da pasta
mostrada).

Uma cópia colada onde o nome já existe recebe `nome (copy).ext`,
`nome (copy 2).ext`, ...  Uma entrada recortada fica acinzentada até ser
colada (ela é movida, não copiada).  Uma pasta é copiada com tudo o que
há nela, um link simbólico como link; mover para outro disco é copiar e
depois remover.  Um documento aberto no editor acompanha o arquivo
quando ele (ou a pasta dele) é renomeado ou movido.  A pasta do projeto
e a pasta pessoal não são movidas.

Properties mostra o nome, o tipo, o local, o tamanho (de uma pasta: de
tudo o que há nela, contado por no máximo dois segundos), a data da
última alteração, as permissões e o dono.

A busca procura em tudo o que há sob a pasta mostrada, em todas as
subpastas (as mais próximas primeiro, enquanto o editor está ocioso; a
contagem fica à direita da linha): as pastas e os arquivos cujo caminho
combina (busca aproximada, como no Alt+P), os melhores primeiro,
mostrados com o caminho (as letras que combinaram sublinhadas).  Enter
numa pasta encontrada entra nela, num arquivo o abre; Right (no fim do
que foi digitado, ou na lista) a mostra na árvore; o menu funciona nelas
como na árvore.  As entradas ocultas ficam de fora, a não ser com Hidden
ligado.

### Como os arquivos são salvos

Salvar grava um arquivo novo ao lado do antigo e o renomeia por cima
dele, mantendo as permissões: o arquivo antigo fica inteiro até o novo
estar completo.  O texto é gravado na codificação e no fim de linha do
documento (seção 10).

Se outro programa alterar um arquivo aberto, o editor avisa (uma vez
por alteração) e, quando o documento não tem alterações não salvas,
pergunta se deve recarregá-lo.

## 9. O projeto

A **pasta do projeto** limita o Open File in Project e o Search in
Project, e é onde Open e Save As começam.  Ela aparece na barra de
status.

Ela é:

1. a pasta dada na linha de comando (`cedit PASTA`); senão,
2. a pasta escolhida em File > Open Project Folder; senão,
3. a pasta em que o editor foi iniciado (pelo menu do sistema costuma
   ser a pasta pessoal; escolha o projeto com Alt+O).

O **projeto** que as buscas cobrem é o repositório Git que contém a
pasta do projeto (a primeira pasta acima com `.git`), ou então a própria
pasta do projeto.  O nome dele aparece na moldura dos diálogos de busca.

### Open Project Folder (Alt+O)

Também abre com um clique no projeto na barra de status.  Funciona como
o Open File, mas lista só pastas.  A primeira entrada, `./ (open here)`,
representa a pasta mostrada.

- Digitar filtra a lista; Cima e Baixo escolhem.
- Enter numa pasta entra nela; em `..`, sobe.
- Enter em `./` faz da pasta mostrada o projeto.
- Alt+Enter faz da pasta escolhida o projeto, sem entrar nela.
- Backspace com Search vazio, ou Alt+Cima, sobe; `nome/`, `~/`, `/` e
  caminhos digitados navegam.
- O botão **Open This Folder** faz da pasta mostrada o projeto.

### Open File in Project (Alt+P)

À esquerda, a lista de arquivos do projeto; à
direita, uma prévia do arquivo escolhido (com realce; um binário é
indicado como tal).

- Digitar filtra a lista por busca aproximada no caminho: as letras na
  ordem, com espaços separando palavras que precisam todas casar.
  Casar no início de um nome, letras juntas e casar no nome do arquivo
  contam mais.  Uma letra maiúscula torna a busca sensível a
  maiúsculas.
- Os arquivos são encontrados enquanto o editor está ocioso, um pedaço
  por vez, e a lista vai enchendo ("(running)" aparece enquanto isso).
- Ficam de fora: arquivos e pastas ocultos (`.git`, `.cache`, …); o que
  os `.gitignore` e `.ignore` de cada pasta, e o `.git/info/exclude`,
  excluem; links para pastas.

### Search in Project (Alt+/)

Procura no texto de todos os arquivos do
projeto uma expressão regular (ou um texto simples, se o que foi
digitado não tem caracteres especiais).  Cada ocorrência aparece como
`caminho:linha`, e a prévia mostra a linha no meio.

Como a busca funciona:

- Em uma thread por núcleo do processador (no máximo 8), e o editor
  continua respondendo.  Uma nova busca cancela a anterior na hora.
- Cada arquivo é lido um bloco por vez (256 KB; o primeiro, 8 KB).  Os
  arquivos nunca são carregados inteiros, nunca mapeados e nunca
  gravados, então um arquivo de qualquer tamanho pode ser buscado.
- A codificação de cada arquivo é detectada como na abertura (um
  arquivo grande, pelo primeiro bloco).  UTF-8 é buscado como está;
  outras codificações são decodificadas um bloco por vez.  O CR LF é
  mantido, e `$` casa antes dele.
- Um arquivo é tratado como binário, e deixado de lado, a partir do
  primeiro byte zero.
- Uma expressão regular que demoraria demais numa linha (como `.*` sobre
  megabytes) pula essa linha em vez de travar.

### Teclas nos dois diálogos de busca

| Tecla | Ação |
|---|---|
| Cima, Baixo, PgUp, PgDn, Ctrl+P, Ctrl+N, a roda, um clique | Escolhe uma entrada |
| Enter, clique duplo | Abre o arquivo (na linha encontrada) |
| A roda sobre a prévia | Rola a prévia |
| Esc | Fecha |

Nenhum programa externo (grep, rg, find, git) é executado: tudo é código
C99 do próprio editor.

## 10. Codificações e fins de linha

Dentro do editor o texto é UTF-8.  Um arquivo em outra codificação é
decodificado na leitura e codificado de novo ao salvar.  Codificações
convertidas pelo próprio editor: UTF-8, UTF-8 com BOM, UTF-16 LE, UTF-16 BE,
Windows-1252, ISO-8859-1 e ISO-8859-15.  O resto da lista (UTF-16 e
UTF-32 com BOM, UTF-7, as outras ISO-8859, as code pages do Windows, da
IBM e do Mac, KOI8, Shift_JIS, EUC-JP, ISO-2022-JP, HZ, GB18030, GBK, Big5,
EUC-KR, …) também é convertido pelo editor, por tabelas que ficam no
código-fonte (`src/core/enc_data.c`): nenhuma biblioteca é necessária.
Essas nunca são detectadas: só podem ser escolhidas.

Como a codificação é detectada ao abrir um arquivo:

1. um BOM indica UTF-8, UTF-16 LE ou UTF-16 BE;
2. senão, UTF-16 sem BOM é reconhecido pelos bytes zero, e é salvo sem
   BOM;
3. senão UTF-8, se os bytes forem UTF-8 válido (num arquivo de 32 MB ou
   mais, os primeiros 64 KB são verificados);
4. senão, pelos primeiros 64 KB, japonês (Shift_JIS, EUC-JP), chinês
   (GB18030, Big5), coreano (EUC-KR) ou cirílico (windows-1251, KOI8-R,
   IBM866): os bytes precisam ser válidos nela, e os caracteres que dão
   precisam ser os mais usados da língua (kana, os hanzi ou hangul mais
   frequentes, as letras mais frequentes do russo).  Um texto de poucos
   caracteres pode não ser reconhecido;
5. senão Windows-1252, ou ISO-8859-1 quando nenhum byte está entre 80h e
   9Fh.  ISO-8859-15 nunca é adivinhada; só pode ser escolhida.

Um arquivo aberto e salvo sem alterações é gravado byte a byte igual,
em qualquer codificação.

A codificação aparece na barra de status.  O botão dela, ou File >
Encoding…, abre um popup com duas ações:

- **Reopen with encoding…**: relê o arquivo na codificação escolhida
  (só quando não há alterações não salvas);
- **Convert to encoding…**: mantém o texto, e o próximo salvamento grava
  na codificação escolhida.

Cada uma abre a lista das codificações, com a atual escolhida.  Digitar
em **Find:** filtra a lista (busca aproximada, como no Alt+P); as setas
escolhem, e Enter ou um clique duplo aplica; Esc fecha.  Um documento sem
nome não tem o que reler: a lista abre direto, para converter.

Se o texto tem um caractere que uma codificação de 8 bits não
representa, o salvamento não grava nada: o caractere fica selecionado e
a linha dele é informada.

O fim de linha (LF ou CR LF) é detectado por arquivo e aparece na barra
de status; um clique nele alterna.

## 11. O menu View

Cada opção vale por documento, e mudá-la também a guarda nas
configurações para os próximos documentos e a próxima execução.

| Item | Tecla | Padrão | Significado |
|---|---|---|---|
| Go to File… | Ctrl+P | | Troca entre os documentos abertos |
| Go to Line:Column… | Ctrl+G | | |
| Go to Matching Bracket | Ctrl+] | | |
| Go Back / Go Forward | Alt+Esquerda / Alt+Direita | | |
| Focus Statusbar | | | Foco do teclado na barra de status |
| Word Wrap | Alt+Z | desligado | Quebra as linhas longas na borda da janela |
| Show Tabs | | desligado | Desenha cada tab como uma linha terminada em seta |
| Show Indent Guides | | ligado | Uma linha fina em cada nível de indentação |
| Show Line Ends | | ligado | Marca o fim de cada linha |
| Show End of File | | ligado | Marca o fim do texto |
| Line Numbers | Alt+N | ligado | Números na margem |
| Relative Line Numbers | Alt+R | desligado | A distância até a linha do cursor |
| Syntax Highlighting | | ligado | |
| Highlight Current Line | | ligado | |
| Highlight Matching Brackets | | ligado | O parêntese ou a palavra de bloco no cursor e seu par |
| Zoom In / Out / Reset | Ctrl++ / Ctrl+- / Ctrl+0 | | só no xcedit |

O menu Edit tem mais duas opções: **Auto Indent** (ligado) e **Backspace
Removes a Whole Indent** (ligado).

Com os números de linha desligados, as linhas nem são contadas; a barra
de status mostra só a coluna, e a barra de rolagem a posição em bytes.

## 12. Realce de sintaxe e linguagens

158 linguagens têm realce, de C, Python e JavaScript a COBOL, Ada,
Fortran, Lisp e Alef.  A linguagem de um documento é escolhida, nesta
ordem, pelo nome do arquivo, pela extensão, pelo fim do caminho, pela
linha `#!` e pelas primeiras linhas.  A barra de status a mostra.

Cada linguagem é um pequeno arquivo C em `src/app/langs/` que lista
extensões, palavras-chave, tipos, comentários, strings e blocos.  Para
acrescentar uma, copie `docs/lang_template.c`, acrescente uma linha em
`src/app/langs/all.h` e o objeto dela em `LANGS` no Makefile (o `make
check-langs` avisa de um esquecido) e confira o resultado com `cedit --lex ARQUIVO`.
O guia completo é o [LANGUAGES.md](LANGUAGES.md).

Não há servidor de linguagem (LSP).

## 13. Arquivos grandes

O editor abre arquivos de qualquer tamanho rapidamente:

- Um arquivo de 32 MB ou mais é mapeado na memória em vez de lido: abre
  na hora e só as partes mostradas são lidas do disco.  Um arquivo com
  fim de linha CR LF é convertido, o que o copia.
- O texto é uma piece table numa árvore balanceada: editar, desfazer e
  localizar levam tempo O(log n), e nada copia o texto.
- Os números de linha são contados aos poucos, enquanto o editor espera
  teclas.  Até a contagem chegar ao topo da tela, a margem mostra, em
  tom apagado, as distâncias até o cursor, e a barra de status o
  progresso (`…37%:1`).  O Go to Line conta o que precisa na hora.
- Linhas muito longas (megabytes numa linha só) se movem, desenham e
  rolam sem travar.
- O realce olha no máximo 64 KB antes da tela.
- Um arquivo mapeado nunca é gravado por dentro: salvar sempre grava um
  arquivo novo.  Se outro programa truncar o arquivo enquanto está
  mapeado, as páginas perdidas viram zeros e o editor avisa, em vez de
  cair.

Para abrir um arquivo enorme o mais rápido possível, use `-n` (sem
números de linha).

## 14. A janela (xcedit)

- **Zoom**: Ctrl e + ou - (ou Ctrl e a roda); Ctrl+0 volta ao normal.
  O tamanho fica nas configurações (`font_size`).
- **Fonte**: a FantasqueSansMono vem embutida.  Outra pode ser definida
  nas configurações (`font`): um arquivo (`"~/fonts/Iosevka.ttf"`; .ttf,
  .otf ou .ttc) ou o nome de uma família (`"JetBrains Mono"`,
  `"JetBrains Mono:Bold"`).  As famílias são procuradas pelo próprio
  editor, lendo as tabelas de nomes das fontes em
  `~/.local/share/fonts`, `~/.fonts`, `$XDG_DATA_HOME/fonts` e na pasta
  `fonts` de cada item de `$XDG_DATA_DIRS`.  A célula tem a largura do
  `M` da fonte, então espera-se uma fonte monoespaçada.  Se a fonte não
  for encontrada ou não puder ser lida, o editor avisa e mantém a atual.
- **Área de transferência**: Ctrl+V e Shift+Insert colam o que outros
  programas copiaram; o que o editor copia vai para a área de
  transferência do sistema.
- As teclas e o mouse se comportam como no terminal.  Nos diálogos,
  Ctrl+H mostra ou esconde os arquivos ocultos (Alt+H no terminal).
- **Tela de toque** (Wayland): um toque é um clique (dois toques, um
  clique duplo); o dedo arrastado rola o texto, as listas e os menus;
  segurado e solto, abre o menu de contexto; segurado e arrastado,
  seleciona.  Dois dedos aproximados ou afastados mudam o zoom do texto
  (como Ctrl e + ou -); arrastados juntos, rolam o texto.  No X11 o
  sistema transforma os toques em cliques do mouse.
- **O terminal**: veja a [seção 2](#2-iniciar-o-editor) (`-f`).

## 15. Configurações

As configurações ficam em `$XDG_CONFIG_HOME/cedit/settings.json` (por
padrão `~/.config/cedit/settings.json`), compartilhadas pelo cedit e
pelo xcedit.  File > Preferences abre esse arquivo no editor, criando-o
com os valores atuais se não existir.  Ao salvá-lo, ele é verificado e
aplicado na hora aos documentos abertos.

Ligar ou desligar uma opção nos menus ou na barra Find também grava o
arquivo.  Toda gravação é conferida: o JSON é analisado e cada valor
lido de volta, antes e depois de gravar.  Um arquivo que não é JSON
válido não é usado: o editor avisa "Invalid Settings: Invalid JSON", usa
os padrões e guarda uma cópia como `settings.json.invalid` antes de
gravar por cima.

| Nome | Tipo | Padrão | Significado |
|---|---|---|---|
| `word_wrap` | bool | false | View > Word Wrap |
| `auto_indent` | bool | true | Edit > Auto Indent |
| `show_tabs` | bool | false | View > Show Tabs |
| `show_line_ends` | bool | true | View > Show Line Ends |
| `show_end_of_file` | bool | true | View > Show End of File |
| `line_numbers` | bool | true | View > Line Numbers |
| `relative_line_numbers` | bool | false | View > Relative Line Numbers |
| `highlight_current_line` | bool | true | View > Highlight Current Line |
| `syntax_highlighting` | bool | true | View > Syntax Highlighting |
| `match_brackets` | bool | true | View > Highlight Matching Brackets |
| `backspace_unindents` | bool | true | Edit > Backspace Removes a Whole Indent |
| `indent_guides` | bool | true | View > Show Indent Guides |
| `find_match_case` | bool | false | Find: Match Case |
| `find_whole_word` | bool | false | Find: Whole Word |
| `find_use_regex` | bool | false | Find: Use Regex |
| `show_hidden_files` | bool | false | Open e Save As: mostram os arquivos ocultos |
| `expand_tabs` | bool | true | Tab insere espaços, num texto ainda sem indentação |
| `tab_width` | 1–8 | 4 | Largura do tab, e a indentação de um texto ainda sem indentação |
| `ruler_column` | 0–9999 | 0 | Desenha uma régua depois dessa coluna (80: depois da 80ª; 0: nenhuma) |
| `font_size` | 0–200 | 0 | Tamanho do texto do xcedit em pixels (0: o padrão) |
| `font` | string | `""` | Fonte do xcedit: um arquivo ou uma família; `""` é a embutida |

Um arquivo que já tem indentação a mantém: `expand_tabs` e `tab_width`
só decidem como um texto novo ou sem indentação é indentado.

Exemplo:

```json
{
  "word_wrap": false,
  "line_numbers": true,
  "relative_line_numbers": true,
  "tab_width": 8,
  "expand_tabs": false,
  "ruler_column": 80,
  "font_size": 18,
  "font": "JetBrains Mono"
}
```

## 16. Referência de teclas

Em alguns terminais, Alt+letra precisa ser habilitado ("meta sends
escape").

### Arquivos

| Tecla | Ação |
|---|---|
| Ctrl+N | New File |
| Ctrl+O | Open File |
| Alt+O | Open Project Folder |
| Alt+P | Open File in Project |
| Ctrl+E | Explorer (gerenciador de arquivos) |
| Ctrl+S | Save |
| Ctrl+Shift+S | Save As |
| Ctrl+W | Close File |
| Ctrl+Q | Exit |
| Alt+N (no Open, Save As, Open Project Folder) | New Folder (pasta nova) |
| Del (nos mesmos, na lista) | Move a pasta ou o arquivo para a Lixeira |

### Edição

| Tecla | Ação |
|---|---|
| Ctrl+Z / Ctrl+Y, Ctrl+Shift+Z | Desfazer / Refazer |
| Ctrl+X / Ctrl+C / Ctrl+V | Recortar / Copiar / Colar |
| Shift+Delete / Ctrl+Insert / Shift+Insert | Recortar / Copiar / Colar |
| Ctrl+A | Selecionar tudo |
| Ctrl+L | Selecionar a linha |
| Shift+Alt+Direita / Shift+Alt+Esquerda | Expand / Shrink Selection |
| Ctrl+D | Add Next Occurrence |
| Shift+Alt+Cima / Shift+Alt+Baixo | Cursor acima / abaixo |
| Alt+clique | Acrescenta ou remove um cursor |
| Esc | Volta a um cursor |
| Ctrl+Space | Complete Word |
| Alt+\| | Filter Through Command |
| Ctrl+Backspace / Ctrl+Delete | Apaga uma palavra |
| Ctrl+Shift+H / Ctrl+Shift+G | Apaga até o início / fim da linha |
| Alt+Cima / Alt+Baixo | Move linhas para cima / baixo |
| Tab / Shift+Tab | Indenta / desindenta (linhas selecionadas) |
| Insert | Liga ou desliga o modo de sobrescrever |
| Alt+M / Alt+J | Set Mark / Jump to Mark |

### Busca

| Tecla | Ação |
|---|---|
| Ctrl+F | Find |
| Ctrl+R | Replace |
| F3, Enter no Find | Próxima ocorrência |
| Enter no Replace | Substitui e vai à próxima |
| Ctrl+Alt+Enter no Replace | Replace All |
| Alt+/ | Search in Project |
| Esc | Fecha a barra |

### Navegação e visualização

| Tecla | Ação |
|---|---|
| Ctrl+G | Go to Line:Column |
| Ctrl+P | Go to File (documentos abertos) |
| Ctrl+] | Go to Matching Bracket |
| Alt+Esquerda / Alt+Direita | Go Back / Go Forward |
| Ctrl+Cima / Ctrl+Baixo | Rola uma linha |
| Alt+Z | Word Wrap |
| Alt+N | Line Numbers |
| Alt+R | Relative Line Numbers |
| Ctrl++ / Ctrl+- / Ctrl+0 | Zoom In / Out / Reset (xcedit) |
| F10, Alt+letra | Abre um menu |

## 17. Arquivos e ambiente

| Caminho ou variável | Uso |
|---|---|
| `$XDG_CONFIG_HOME/cedit/settings.json` | As configurações (padrão `~/.config/cedit/settings.json`) |
| `settings.json.invalid` | Cópia de um arquivo de configurações que não era JSON válido |
| `$HOME` | O `~` nos caminhos e nos diálogos de abrir |
| `$XDG_DATA_HOME`, `$XDG_DATA_DIRS` | Onde o xcedit procura as famílias de fontes |
| `XCEDIT_KEYS` | Testes: teclas que o xcedit digita ao iniciar (`~X` Alt+X, `^X` Ctrl+X, `\|` Enter, `!` Esc, `{x,y}` um clique numa célula, `{x,y>x2,y2}` um arrasto) |
| `XCEDIT_SNAPSHOT=arquivo.ppm` | Testes: o xcedit grava cada quadro como imagem |
| `XCEDIT_TRACE=arquivo` | Testes: o xcedit grava cada evento que recebe |
| `.gitignore`, `.ignore`, `.git/info/exclude` | Arquivos deixados de fora das buscas do projeto |
| `$XDG_DATA_HOME/Trash` (`~/.local/share/Trash`), `.Trash-UID` | A Lixeira para onde as pastas e os arquivos são movidos |

### Código de saída

0 em caso de sucesso; 1 quando o terminal ou a janela não podem ser
abertos, ou o `--lex` falha.

### Organização das fontes

| Pasta | Conteúdo |
|---|---|
| `src/core` | O texto (piece table, desfazer), codificações, expressões regulares, memória |
| `src/tui` | O buffer da tela e os widgets da interface em modo imediato |
| `src/app` | O editor: menus, área de texto, barra de status, diálogos, busca, linguagens |
| `src/app/langs` | Um arquivo por linguagem, listados em `all.h` |
| `src/platform` | Arquivos e pastas, o terminal, threads (`posix/`), a janela (`glfw/`) |
| `docs` | Este manual, a página de manual, o guia de linguagens |
