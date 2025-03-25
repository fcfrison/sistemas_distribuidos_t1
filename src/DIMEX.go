package DIMEX

import (
	PP2PLink "SD/PP2PLink"
	"fmt"
	"strconv"
	"strings"
)

// ------------------------------------------------------------------------------------
// ------- principais tipos
// ------------------------------------------------------------------------------------

type State int // enumeracao dos estados possiveis de um processo
const (
	noMX State = iota
	wantMX
	inMX
)

type dmxReq int // enumeracao dos estados possiveis de um processo
const (
	ENTER dmxReq = iota
	EXIT
)

type dmxResp struct { // mensagem do módulo DIMEX infrmando que pode acessar - pode ser somente um sinal (vazio)
	// mensagem para aplicacao indicando que pode prosseguir
}

type DIMEX_Module struct {
	Req       chan dmxReq  // canal para receber pedidos da aplicacao (REQ e EXIT)
	Ind       chan dmxResp // canal para informar aplicacao que pode acessar
	addresses []string     // endereco de todos, na mesma ordem
	id        int          // identificador do processo - é o indice no array de enderecos acima
	st        State        // estado deste processo na exclusao mutua distribuida
	waiting   []bool       // processos aguardando tem flag true
	lcl       int          // relogio logico local
	reqTs     int          // timestamp local da ultima requisicao deste processo
	nbrResps  int
	dbg       bool

	Pp2plink *PP2PLink.PP2PLink // acesso aa comunicacao enviar por PP2PLinq.Req  e receber por PP2PLinq.Ind
}

// ------------------------------------------------------------------------------------
// ------- inicializacao
// ------------------------------------------------------------------------------------

func NewDIMEX(_addresses []string, _id int, _dbg bool) *DIMEX_Module {

	p2p := PP2PLink.NewPP2PLink(_addresses[_id], _dbg)

	dmx := &DIMEX_Module{
		Req: make(chan dmxReq, 1),
		Ind: make(chan dmxResp, 1),

		addresses: _addresses,
		id:        _id,
		st:        noMX,
		waiting:   make([]bool, len(_addresses)),
		lcl:       0,
		reqTs:     0,
		dbg:       _dbg,

		Pp2plink: p2p}

	for i := 0; i < len(dmx.waiting); i++ {
		dmx.waiting[i] = false
	}
	dmx.Start()
	dmx.outDbg("Init DIMEX!")
	return dmx
}

// ------------------------------------------------------------------------------------
// ------- nucleo do funcionamento
// ------------------------------------------------------------------------------------

func (module *DIMEX_Module) Start() {

	go func() {
		for {
			select {
			case dmxR := <-module.Req: // vindo da  aplicação
				if dmxR == ENTER {
					module.outDbg("app pede mx")
					module.handleUponReqEntry() // ENTRADA DO ALGORITMO

				} else if dmxR == EXIT {
					module.outDbg("app libera mx")
					module.handleUponReqExit() // ENTRADA DO ALGORITMO
				}

			case msgOutro := <-module.Pp2plink.Ind: // vindo de outro processo
				//fmt.Printf("dimex recebe da rede: ", msgOutro)
				if strings.Contains(msgOutro.Message, "respOK") {
					module.outDbg("         <<<---- responde! " + msgOutro.Message)
					module.handleUponDeliverRespOk(msgOutro) // ENTRADA DO ALGORITMO

				} else if strings.Contains(msgOutro.Message, "reqEntry") {
					module.outDbg("          <<<---- pede??  " + msgOutro.Message)
					module.handleUponDeliverReqEntry(msgOutro) // ENTRADA DO ALGORITMO

				}
			}
		}
	}()
}

// ------------------------------------------------------------------------------------
// ------- tratamento de pedidos vindos da aplicacao
// ------- UPON ENTRY
// ------- UPON EXIT
// ------------------------------------------------------------------------------------

// handleUponReqEntry é uma função do módulo DIMEX que lida com as solicitações de
// entrada em uma seção crítica.
// A função incrementa o timestamp lógico do módulo, armazena o timestamp da solicitação
// e zera a contagem de respostas.
// Em seguida, a função envia solicitações para todos os outros processos e
// atualiza o estado do módulo para 'querendo entrar na seção crítica'.
func (module *DIMEX_Module) handleUponReqEntry() {
	message := ""
	space := "|"
	module.lcl++
	module.reqTs = module.lcl
	module.nbrResps = 0
	for i := 0; i < len(module.addresses); i++ {
		if i != module.id {
			message = "reqEntry" + space + strconv.Itoa(module.id) + space + strconv.Itoa(module.reqTs)
			module.sendToLink(module.addresses[i], message, space)
		}
	}
	module.st = wantMX
}

// handleUponReqExit é uma função do módulo DIMEX que lida com as solicitações de saída de uma seção crítica.
// A função envia uma resposta de sucesso para todos os processos que estavam esperando para entrar na seção crítica.
// Em seguida, a função atualiza o estado do módulo para 'não na seção crítica' e zera a lista de processos esperando.
func (module *DIMEX_Module) handleUponReqExit() {
	space := "|" // separador de campos
	for i := 0; i < len(module.waiting); i++ {
		if module.waiting[i] {
			module.sendToLink(module.addresses[i], "respOK"+space+strconv.Itoa(module.id), space)
			module.waiting[i] = false
		}
	}
	module.st = noMX
}

// ------------------------------------------------------------------------------------
// ------- tratamento de mensagens de outros processos
// ------- UPON respOK
// ------- UPON reqEntry
// ------------------------------------------------------------------------------------

// handleUponDeliverRespOk é uma função do módulo DIMEX que lida com as respostas
// de sucesso recebidas de outros processos.
// A função incrementa o contador de respostas e, se todas as respostas forem recebidas,
// envia uma resposta para o canal Ind do módulo e atualiza o estado do módulo para 'dentro da seção crítica'.
func (module *DIMEX_Module) handleUponDeliverRespOk(msgOutro PP2PLink.PP2PLink_Ind_Message) {
	module.nbrResps++
	if module.nbrResps == len(module.addresses)-1 {
		module.Ind <- dmxResp{}
		module.st = inMX
	}
}

// handleUponDeliverReqEntry é uma função do módulo DIMEX que lida com as solicitações
// de entrada recebidas de outros processos. A função analisa a mensagem recebida,
// verifica o estado do módulo e o timestamp da solicitação, e envia uma resposta apropriada.
// Se o módulo estiver 'querendo entrar na seção crítica' e o timestamp da solicitação
// for menor que o do módulo, ou se o módulo estiver 'não na seção crítica',
// a função envia uma resposta de sucesso. Caso contrário, o ID do processo é
// adicionado à lista de processos esperando. Se o timestamp da solicitação for
// maior que o timestamp lógico do módulo, o timestamp lógico do módulo é atualizado.
func (module *DIMEX_Module) handleUponDeliverReqEntry(msgOutro PP2PLink.PP2PLink_Ind_Message) {
	input := msgOutro.Message
	parts := strings.Split(input, "|")
	otID, _ := strconv.Atoi(parts[1])
	otRts, _ := strconv.Atoi(parts[2])
	if (module.st == wantMX && !before(module.id, module.reqTs, otID, otRts)) || module.st == noMX {
		module.sendToLink(module.addresses[otID], "respOK", "|")
	} else {
		module.waiting[otID] = true
	}
	if otRts > module.lcl {
		module.lcl = otRts
	}
}

// ------------------------------------------------------------------------------------
// ------- funcoes de ajuda
// ------------------------------------------------------------------------------------

func (module *DIMEX_Module) sendToLink(address string, content string, space string) {
	module.outDbg(space + " ---->>>>   to: " + address + "     msg: " + content)
	module.Pp2plink.Req <- PP2PLink.PP2PLink_Req_Message{
		To:      address,
		Message: content}
}

func before(oneId, oneTs, othId, othTs int) bool {
	if oneTs < othTs {
		return true
	} else if oneTs > othTs {
		return false
	} else {
		return oneId < othId
	}
}

func (module *DIMEX_Module) outDbg(s string) {
	if module.dbg {
		fmt.Println(". . . . . . . . . . . . [ DIMEX : " + s + " ]")
	}
}
