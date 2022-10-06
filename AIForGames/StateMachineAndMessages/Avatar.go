package main

type Avatar struct {
	Entity
	StateMachine
}

func (avatar *Avatar) Init() {
	avatar.Entity = CreateEntity()
	avatar.StateMachine = createAvatarStateMachine()
	avatar.StateMachine.owner = avatar
	RegisterEntity(avatar)
}

func (avatar *Avatar) Update() {
	avatar.Entity.Update()
	avatar.StateMachine.Update()
}

func (avatar *Avatar) Destroy() {}

func (avatar *Avatar) OnMsg(source uint64, msg Msg) {
	avatar.StateMachine.OnMsg(source, msg)
}

func createAvatarStateMachine() StateMachine {
	return StateMachine{
		currentState: &AvatarIdleState{},
		globalState:  &AvatarGlobalState{},
	}
}

func CreateAvatar() *Avatar {
	avatar := Avatar{}
	avatar.Init()
	return &avatar
}
