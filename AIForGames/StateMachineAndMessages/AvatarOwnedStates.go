package main

import (
	"fmt"
)

type AvatarIdleState struct{}
type AvatarAttackState struct {
	source      uint64
	destination uint64
}
type AvatarFleeState struct{}
type AvatarGlobalState struct{}

func (avatarIdleState *AvatarIdleState) OnEnter(owner IBase) {
	entity := GetEntity(owner)
	if entity == nil {
		return
	}
	fmt.Println(fmt.Sprintf("Avatar id:%v in position:%v enter idling", entity.GetID(), entity.GetPosition()))
}

func (avatarIdleState *AvatarIdleState) OnExit(owner IBase) {
	entity := GetEntity(owner)
	if entity == nil {
		return
	}
	fmt.Println(fmt.Sprintf("Avatar id:%v in position:%v exit idling", entity.GetID(), entity.GetPosition()))
}

func (avatarIdleState *AvatarIdleState) OnUpdate(owner IBase) {
	entity := GetEntity(owner)
	if entity == nil {
		return
	}
	fmt.Println(fmt.Sprintf("Avatar id:%v in position:%v idling", entity.GetID(), entity.GetPosition()))
}

func (avatarIdleState *AvatarIdleState) OnMsg(owner IBase, source uint64, msg Msg) {}

func (avatarAttackState *AvatarAttackState) OnEnter(owner IBase) {
	entity := GetEntity(owner)
	if entity == nil {
		return
	}
	fmt.Println(fmt.Sprintf("Avatar id:%v in position:%v enter attacking", entity.GetID(), entity.GetPosition()))
}

func (avatarAttackState *AvatarAttackState) OnExit(owner IBase) {
	entity := GetEntity(owner)
	if entity == nil {
		return
	}
	fmt.Println(fmt.Sprintf("Avatar id:%v in position:%v exit attacking", entity.GetID(), entity.GetPosition()))
}

func (avatarAttackState *AvatarAttackState) OnUpdate(owner IBase) {
	entity := GetEntity(owner)
	if entity == nil {
		return
	}
	fmt.Println(fmt.Sprintf("Avatar id:%v in position:%v attacking id:%v", entity.GetID(), entity.GetPosition(), avatarAttackState.destination))
}

func (avatarAttackState *AvatarAttackState) OnMsg(owner IBase, source uint64, msg Msg) {
	entity := GetEntity(owner)
	if entity == nil {
		return
	}
	sourceEntity := GetProperty(source)
	if sourceEntity == nil {
		return
	}
	fmt.Println(fmt.Sprintf("id:%v in position:%v attack id:%v", entity.GetID(), entity.GetPosition(), sourceEntity.GetID()))
}

func (avatarFleeState *AvatarFleeState) OnEnter(owner IBase) {
	entity := GetEntity(owner)
	if entity == nil {
		return
	}
	fmt.Println(fmt.Sprintf("Avatar id:%v in position:%v enter fleeing", entity.GetID(), entity.GetPosition()))
}

func (avatarFleeState *AvatarFleeState) OnExit(owner IBase) {
	entity := GetEntity(owner)
	if entity == nil {
		return
	}
	fmt.Println(fmt.Sprintf("Avatar id:%v in position:%v exit fleeing", entity.GetID(), entity.GetPosition()))
}

func (avatarFleeState *AvatarFleeState) OnUpdate(owner IBase) {
	entity := GetEntity(owner)
	if entity == nil {
		return
	}
	fmt.Println(fmt.Sprintf("Avatar id:%v in position:%v fleeing", entity.GetID(), entity.GetPosition()))
}

func (avatarFleeState *AvatarFleeState) OnMsg(owner IBase, source uint64, msg Msg) {}

func (avatarGlobalState *AvatarGlobalState) OnEnter(owner IBase) {}

func (avatarGlobalState *AvatarGlobalState) OnExit(owner IBase) {}

func (avatarGlobalState *AvatarGlobalState) OnUpdate(owner IBase) {}

func (avatarGlobalState *AvatarGlobalState) OnMsg(owner IBase, source uint64, msg Msg) {}
