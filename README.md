# Bomba MICSA

## Ideia do projeto

A ideia central do projeto é desenvolver um bomb defuser com recurso a uma placa STM32 (NUCLEO-F411RE) e alguns outros periféricos.

De modo a equilibrar a dificuldade e o tempo, chegou-se à conclusão que um código de 3 dígitos e 120 segundos de tempo, aliado de certas dicas que serão abordadas a seguir, é o suficiente para gerar uma experiência agradável.

Para que não seja impossível adivinhar o código, é importante ajudar o jogador em certas fases do jogo. Note-se que, caso o utilizador falhe uma tentativa, será penalizado por isso, sendo alvo de uma redução do tempo restante, mas por outro lado, é lhe dada uma pista, mencionado se o valor introduzido está abaixo ou acima do valor correto.

Foi também introduzido um sistema de penalização: quanto mais o jogador falha, maior é a redução do tempo, bem como um sistema de identificação de teclas inválidas, i.e, teclas não-numéricas, uma vez que, por limitação de hardware, não há possibilidade de apagar um valor introduzido, o utilizador seria prejudicado se eventualmente pressionasse uma tecla não-numérica, uma vez que o código estaria automaticamente errado.

## Como funciona o código

Primeiramente, é importante referir que o código desenvolvido segue as regras de clean code, tendo em especial atenção a divisão de responsabilidades (envolvendo a separação de código por periférico usado, i.e, um ficheiro de código para cada periférico, sempre que possível/oportuno), o que torna o código não só mais fácil de ler e compreender, mas também de identificar possíveis erros (ajuda o debugging). Foi dada também bastante importância à gestão de memory leaks, garantindo que não ocorrem.

A gestão do tempo é essencial neste projeto, pelo que foi utilizado um timer interno da placa, que gera um interrupt a cada segundo que passa. Para alcançar este objetivo, foi utilizado um Prescaler de 8399 com um Counter Period de 9999. A função relativa a este interrupt apenas decrementa a variável global time_remaining. Note-se que à medida que o tempo passa, o led e o buzzer piscam e apitam, respetivamente, mais rápido, sendo a velocidade determinada com base no tempo restante.

O jogo começa por gerar o código, sendo gerado um dígito de cada vez, com recurso à função rand(). De modo a garantir que o código gerado é sempre aleatório, foi utilizado o Real-Time Clock (RTC) da placa, para gerar a seed de srand().

De seguida, o jogo está pronto e o utilizador pode começar a introduzir o código. As teclas são lidas do keypad com recurso ao seguinte algoritmo da função read_keypad():

    1. Reset: Coloca todas as linhas em HIGH;
    2. Ativação: Coloca apenas a linha atual em LOW;
    3. Leitura: Verifica qual coluna respondeu.

A função handle_keypress() é responsável por verificar se a tecla introduzida é válida, e se o número de dígitos introduzidos é igual ao tamanho do código (i.e, 3 dígitos). Em caso afirmativo, é chamada a função checkCodeInput(), que verifica se o código introduzido corresponde ao correto.

Caso o código esteja errado, é chamada a função applyPenalty(). O penalty_multiplier inicia a 1.0, mas é incrementado por 0.5 cada vez que é aplicada uma penalidade (i.e, cada vez que a função applyPenalty é chamada). A penalidade é então calculada com base na penalidade inicial (5 segundos) * penalty_multiplier. A função giveHint() é responsável por fornecer a pista ao jogador. 

Quando o jogo atual termina, i.e, o código foi adivinhado ou o tempo terminou, é chamada a função endgame_routine(), onde é tratada a mensagem final e a alteração da cor do led (em caso de vitória). São chamadas as funções wait_for_restart() que espera que a tecla '#' seja pressionada para começar um novo jogo e caso isso aconteça, a função reset_game(), que trata de restaurar as variáveis aos seus valores iniciais e gerar um novo código.
