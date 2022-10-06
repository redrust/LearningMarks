package main

type IState interface {
	OnEnter(IBase)
	OnExit(IBase)
	OnUpdate(IBase)
	OnMsg(IBase, uint64, Msg)
}

type StateMachine struct {
	currentState  IState
	globalState   IState
	previousState IState
	owner         IBase
}

func (stateMachine *StateMachine) Init() {}

func (stateMachine *StateMachine) Update() {
	if stateMachine.currentState != nil {
		stateMachine.currentState.OnUpdate(stateMachine.owner)
	}
	if stateMachine.globalState != nil {
		stateMachine.globalState.OnUpdate(stateMachine.owner)
	}
}

func (stateMachine *StateMachine) OnMsg(source uint64, msg Msg) {
	if stateMachine.currentState != nil {
		stateMachine.currentState.OnMsg(stateMachine.owner, source, msg)
	}
	if stateMachine.globalState != nil {
		stateMachine.globalState.OnMsg(stateMachine.owner, source, msg)
	}
}

func (stateMachine *StateMachine) Destroy() {}

func (stateMachine *StateMachine) SetCurrentState(targetState IState) {
	if stateMachine.currentState != targetState {
		stateMachine.previousState = stateMachine.currentState
		stateMachine.currentState.OnExit(stateMachine.owner)
		stateMachine.currentState = targetState
		stateMachine.currentState.OnEnter(stateMachine.owner)
	}
}
func (stateMachine *StateMachine) SetGlobalState(targetState IState) {
	if stateMachine.globalState != targetState {
		stateMachine.globalState.OnExit(stateMachine.owner)
		stateMachine.globalState = targetState
		stateMachine.globalState.OnEnter(stateMachine.owner)
	}
}

func (stateMachine *StateMachine) ShiftToPreviousState() {
	if stateMachine.previousState == nil {
		return
	}
	stateMachine.SetCurrentState(stateMachine.previousState)
}
