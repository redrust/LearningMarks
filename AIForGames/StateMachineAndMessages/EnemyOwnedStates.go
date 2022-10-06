package main

import (
	"fmt"
)

type EnemyIdleState struct{}
type EnemyAttackState struct {
	source      uint64
	destination uint64
}
type EnemyFleeState struct{}
type EnemyGlobalState struct{}

func (enemyIdleState *EnemyIdleState) OnEnter(owner IBase) {
	entity := GetEntity(owner)
	if entity == nil {
		return
	}
	fmt.Println(fmt.Sprintf("Enemy id:%v in position:%v enter idling", entity.GetID(), entity.GetPosition()))
}

func (enemyIdleState *EnemyIdleState) OnExit(owner IBase) {
	entity := GetEntity(owner)
	if entity == nil {
		return
	}
	fmt.Println(fmt.Sprintf("Enemy id:%v in position:%v exit idling", entity.GetID(), entity.GetPosition()))
}

func (enemyIdleState *EnemyIdleState) OnUpdate(owner IBase) {
	entity := GetEntity(owner)
	if entity == nil {
		return
	}
	fmt.Println(fmt.Sprintf("Enemy id:%v in position:%v idling", entity.GetID(), entity.GetPosition()))
}

func (enemyIdleState *EnemyIdleState) OnMsg(owner IBase, source uint64, msg Msg) {
	entity := GetEnemy(owner)
	if entity == nil {
		return
	}
	sourceEntity := GetProperty(source)
	if sourceEntity == nil {
		return
	}
	switch msg.mType {
	case MSG_ATTACK:
		{
			entity.SetCurrentState(&EnemyAttackState{
				source:      msg.source,
				destination: msg.destination,
			})
		}
	}
}

func (enemyAttackState *EnemyAttackState) OnEnter(owner IBase) {
	entity := GetEntity(owner)
	if entity == nil {
		return
	}
	fmt.Println(fmt.Sprintf("Enemy id:%v in position:%v enter attacking", entity.GetID(), entity.GetPosition()))
}

func (enemyAttackState *EnemyAttackState) OnExit(owner IBase) {
	entity := GetEntity(owner)
	if entity == nil {
		return
	}
	fmt.Println(fmt.Sprintf("Enemy id:%v in position:%v exit attacking", entity.GetID(), entity.GetPosition()))
}

func (enemyAttackState *EnemyAttackState) OnUpdate(owner IBase) {
	entity := GetEnemy(owner)
	if entity == nil {
		return
	}
	fmt.Println(fmt.Sprintf("Enemy id:%v in position:%v attacking id:%v", entity.GetID(), entity.GetPosition(), enemyAttackState.source))
}

func (enemyAttackState *EnemyAttackState) OnMsg(owner IBase, source uint64, msg Msg) {}

func (enemyFleeState *EnemyFleeState) OnEnter(owner IBase) {
	entity := GetEntity(owner)
	if entity == nil {
		return
	}
	fmt.Println(fmt.Sprintf("Enemy id:%v in position:%v enter fleeing", entity.GetID(), entity.GetPosition()))
}

func (enemyFleeState *EnemyFleeState) OnExit(owner IBase) {
	entity := GetEntity(owner)
	if entity == nil {
		return
	}
	fmt.Println(fmt.Sprintf("Enemy id:%v in position:%v exit fleeing", entity.GetID(), entity.GetPosition()))
}

func (enemyFleeState *EnemyFleeState) OnUpdate(owner IBase) {
	entity := GetEntity(owner)
	if entity == nil {
		return
	}
	fmt.Println(fmt.Sprintf("Enemy id:%v in position:%v fleeing", entity.GetID(), entity.GetPosition()))
}

func (enemyFleeState *EnemyFleeState) OnMsg(owner IBase, source uint64, msg Msg) {}

func (enemyGlobalState *EnemyGlobalState) OnEnter(owner IBase) {}

func (enemyGlobalState *EnemyGlobalState) OnExit(owner IBase) {}

func (enemyGlobalState *EnemyGlobalState) OnUpdate(owner IBase) {}

func (enemyGlobalState *EnemyGlobalState) OnMsg(owner IBase, source uint64, msg Msg) {}
