package main

type Enemy struct {
	Entity
	StateMachine
}

func (enemy *Enemy) Init() {
	enemy.Entity = CreateEntity()
	enemy.StateMachine = createEnemyStateMachine()
	enemy.StateMachine.owner = enemy
	RegisterEntity(enemy)
}

func (enemy *Enemy) Update() {
	enemy.Entity.Update()
	enemy.StateMachine.Update()
}

func (enemy *Enemy) Destroy() {}

func (enemy *Enemy) OnMsg(source uint64, msg Msg) {
	enemy.StateMachine.OnMsg(source, msg)
}

func createEnemyStateMachine() StateMachine {
	return StateMachine{
		currentState: &EnemyIdleState{},
		globalState:  &EnemyGlobalState{},
	}
}

func CreateEnemy() *Enemy {
	enemy := Enemy{}
	enemy.Init()
	return &enemy
}
